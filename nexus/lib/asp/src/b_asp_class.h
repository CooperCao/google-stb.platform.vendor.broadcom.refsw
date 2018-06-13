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

#ifndef B_ASP_CLASS_H
#define B_ASP_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "b_asp_priv.h"

/** @addtogroup B_Asp_Class
 *
 * B_ASP_Class Interface Definition.
 *
 * This class is responsible creating, maintaining, and destroying class-specific
 * constructs, such as class mutexes and class lists (lists of object belonging
 * to each class).
 *
**/


/**
Summary:
The B_ASP_Class structure exists once for each B_ASP class.
**/
typedef struct B_ASP_Class
{
    const char         *pClassName;
    BKNI_MutexHandle    hClassMutex;
    uint32_t            refCount;
    BLST_Q_HEAD(classBaseList, B_ASP_ClassInstance)  listHead;
} B_ASP_Class;

/**
Summary:
The B_ASP_ClassInstance structure exists is embedded inside of the
per-instance data for each B_ASP object.
**/
typedef struct B_ASP_ClassInstance
{
    BLST_Q_ENTRY(B_ASP_ClassInstance)   listNext;
    void                                *pThisInstance;
} B_ASP_ClassInstance;


/**
Summary:
Define and allocate static per-class data.

Description:
This macro allocates a global per-class structure named according to the
specified B_ASP class.

This macro should be placed in the main C source file for the specified
B_ASP class, before any function definitions.

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for the class'
                        per-object data structure.
**/
#define B_ASP_CLASS_DEFINE(className)     B_ASP_Class B_ASP_Class_For_##className


/**
Summary:
Declare external reference to static per-class data.

Description:
This macro makes an external declaration to a B_ASP class' global per-class
structure for specified B_ASP class.

Since each B_ASP class' global per-class structure is defined in the class'
"main" source file, this macro must be used in any other source file that
needs to reference the global per-class structure.

This macro should be placed before any functions, and before any other
B_ASP_CLASS macros.

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for the class'
                        per-object data structure.
**/
#define B_ASP_CLASS_DECLARE(className)    extern B_ASP_Class  B_ASP_Class_For_##className


/**
Summary:
Define per-object data required by B_ASP_CLASS macros.

Description:
This macro adds some variables to the per-object data structure.
These variables are only to be used by the B_ASP_CLASS macros.

This macro must be placed within the class' per-object data structure.

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for
                        the class' per-object data structure.
**/
#define B_ASP_CLASS_DEFINE_INSTANCE_VARS(className)          B_ASP_ClassInstance  bAspClassInstance


/**
Summary:
Perform one-time B_ASP_Class initialization for a class.

Description:
This macro initializes any data structures that are used by the B_ASP_Class macros
for the specified class.

This macro is intended to be invoked by B_Asp_Init().

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for
                        the class' per-object data structure.
**/


NEXUS_Error B_ASP_Class_P_Init(B_ASP_Class *pDesc, const char *pClassName,
                            const char *pFilename, unsigned lineNumber);

#define B_ASP_CLASS_INIT(className, pNexusStatus)   \
        B_ASP_Class_P_Init(&B_ASP_Class_For_##className, #className, __FILE__, __LINE__);


/**
Summary:
Perform one-time B_ASP_Class un-initialization for a class.

Description:
This macro un-initializes (destroys/frees) any data structures that are used by
B_ASP_Class for the specified class.

This macro is intended to be invoked by B_Asp_Uninit().

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for
                        the class' per-object data structure.
**/

NEXUS_Error B_ASP_Class_P_Uninit(B_ASP_Class *pDesc, const char *pClassName,
                              const char *pFilename, unsigned lineNumber);

#define B_ASP_CLASS_UNINIT(className)     \
        B_ASP_Class_P_Uninit(&B_ASP_Class_For_##className, #className, __FILE__, __LINE__);


/**
Summary:
Add an instance to a B_ASP class.

Description:
This macro adds a new object to a class' list of existing objects.  If
necessary.

This macro should be placed in the B_ASP class' "Create" function.

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for the class'
                        per-object data structure.

    pObject -           A pointer to the newly allocated object data
                        structure.

    pNexusStatus -      An optional  pointer to a NEXUS_Error variable that will be
                        set to indicate the completion status of the macro.  The
                        caller can pass NULL if not interested in the completion status.
**/

NEXUS_Error B_ASP_Class_P_AddInstance(B_ASP_Class *pDesc, const char *pClassName, void *pObject, B_ASP_ClassInstance *pClassInstance,
                                   const char *pFilename, unsigned lineNumber);

#define B_ASP_CLASS_ADD_INSTANCE(className, pObject)   \
        B_ASP_Class_P_AddInstance(&B_ASP_Class_For_##className, #className, pObject, &(pObject)->bAspClassInstance, __FILE__, __LINE__);



/**
Summary:
Remove an instance from a B_ASP class.

Description:
This macro removes an existing object from a class' list of existing
objects.  If the object does not belong to the class, it quietly returns
to the caller.

This macro should be place in the B_ASP class' "Destroy" function.

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for the class'
                        per-object data structure.

    pObject -           A pointer to the existing object data structure
                        to be removed from the class.
**/
NEXUS_Error B_ASP_Class_P_RemoveInstance(B_ASP_Class *pDesc, const char *pClassName, void *pObject,
                                      const char *pFilename, unsigned lineNumber);

#define B_ASP_CLASS_REMOVE_INSTANCE(className, pObject)   \
        B_ASP_Class_P_RemoveInstance(&B_ASP_Class_For_##className, #className, pObject, __FILE__, __LINE__);


/**
Summary:
Validate an object pointer and lock the per-class mutex for it's B_ASP class.

Description:
If pObject is found in the B_ASP class's list of object, the per-class mutex will be
locked and the completion status will be NEXUS_SUCCESS.

If pObject is not found, nothing will be locked, and the completion status will be
NEXUS_NOT_AVAILABLE.

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for the class'
                        per-object data structure.

    pObject -           A pointer to an existing object data structure (or maybe not).

    pNexusStatus -      A pointer to a NEXUS_Error variable that will be set to indicate
                        the completion status of the macro.
**/

NEXUS_Error B_ASP_Class_P_LockAndCheckInstance(B_ASP_Class *pDesc, const char *pClassName, void *pObject,
                                            const char *pFilename, unsigned lineNumber);

#define B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(className, pObject)   \
        B_ASP_Class_P_LockAndCheckInstance(&B_ASP_Class_For_##className, #className, pObject, __FILE__, __LINE__);


/**
Summary:
Unlock the per-class mutex for a B_ASP class.

Description:

Arguments:
    className -         The name of the B_ASP class (e.g., B_Asp_Output)
                        It must match a struct tag for the class'
                        per-object data structure.

    pNexusStatus -      A pointer to a NEXUS_Error variable that will be set to indicate
                        the completion status of the macro.  The caller can pass NULL
                        if not interested in the completion status.
**/
NEXUS_Error B_ASP_Class_P_Unlock(B_ASP_Class *pDesc, const char *pClassName, void *pObject,
                              const char *pFilename, unsigned lineNumber);

#define B_ASP_CLASS_UNLOCK(className, pObject)    \
        B_ASP_Class_P_Unlock(&B_ASP_Class_For_##className, #className, pObject, __FILE__, __LINE__);


#ifdef __cplusplus
}
#endif

#endif /* B_ASP_CLASS_H */
