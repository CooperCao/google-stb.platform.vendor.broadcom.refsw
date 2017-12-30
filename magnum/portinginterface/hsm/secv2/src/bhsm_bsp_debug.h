/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#ifndef BHSM_BSP_DEBUG__H_
#define BHSM_BSP_DEBUG__H_

#include "bhsm.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    unsigned dummy;
}BHSM_BspDebugModuleSettings;

#define BHSM_BSP_DEBUG_INPUT 0
#define BHSM_BSP_DEBUG_OUTPUT  (!BHSM_BSP_DEBUG_INPUT)


typedef struct
{
    unsigned numComponents;  /* number of components to hold in database. */
    unsigned numEnums;       /* the number of enumerators to hold in database. */
}BHSM_BspDebugRegisterInit;

typedef struct
{
    unsigned componentIndex;

    char *pComponentName;
    unsigned maxCommandIndex;

}BHSM_BspDebugRegisterComponent;

typedef struct
{
    unsigned componentIndex;
    unsigned commandIndex;

    char *pCommandName;
    unsigned numParametersInput;
    unsigned numParametersOutput;

}BHSM_BspDebugRegisterCommand;

typedef struct
{
    bool io;                   /*  BHSM_BSP_DEBUG_INPUT for input paratmer. BHSM_BSP_DEBUG_OUTPUT for output parameter. */
    unsigned componentIndex;
    unsigned commandIndex;
    unsigned parameterIndex;   /* where its positioned in the list of paramters. */

    unsigned byteOffset;
    unsigned size;             /* size in bytes */
    bool isArray;
    bool isEnum;
    unsigned enumIndex;
    char *pName;               /* valid if isEnum is true */
}BHSM_BspDebugRegisterParamter;


typedef struct
{
    unsigned enumIndex;              /* a inventory index for the enumerator. it will range from 0 to maxElementIndex-1 */
    char *pName;                    /* enumerator name */
    unsigned maxElementIndex;

}BHSM_BspDebugRegisterEnum;

typedef struct
{
    unsigned enumIndex;            /* a inventory index for the enumerator */
    unsigned elementIndex;        /* the index of the element */
    unsigned value;               /* the value of the element. */
    char *pName;                  /* element name */

}BHSM_BspDebugRegisterEnumElement;


/* functions to intialise/unint the module . */
BERR_Code BHSM_BspDebug_Init( BHSM_Handle hHsm, BHSM_BspDebugModuleSettings *pSettings );
BERR_Code BHSM_BspDebug_Uninit( BHSM_Handle hHsm );

/* Print either an Input or output command. These are the principal interface functions */
void BHSM_BspDebug_PrintCommand( BHSM_Handle hHsm, uint32_t *pCommand );
void BHSM_BspDebug_PrintResponse( BHSM_Handle hHsm, uint32_t *pResponse );

/* functions to register components, their commands, and their paramters..  */
BERR_Code BHSM_BspDebug_RegisterInit( BHSM_Handle hHsm, BHSM_BspDebugRegisterInit *pParam );
BERR_Code BHSM_BspDebug_RegisterComponent( BHSM_Handle hHsm, BHSM_BspDebugRegisterComponent *pParam );
BERR_Code BHSM_BspDebug_RegisterCommand( BHSM_Handle hHsm, BHSM_BspDebugRegisterCommand *pParam );
BERR_Code BHSM_BspDebug_RegisterParamter( BHSM_Handle hHsm, BHSM_BspDebugRegisterParamter *pParam );


/* functions to register enumerators.  */
BERR_Code BHSM_BspDebug_RegisterEnum( BHSM_Handle hHsm, BHSM_BspDebugRegisterEnum *pParam );
BERR_Code BHSM_BspDebug_RegisterEnumElement( BHSM_Handle hHsm, BHSM_BspDebugRegisterEnumElement *pParam );


/* the autognerated function that call the above Register functions to prime the internal database.*/
BERR_Code BHSM_BspDebug_LoadAutoGeneratedData( BHSM_Handle hHsm );

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
