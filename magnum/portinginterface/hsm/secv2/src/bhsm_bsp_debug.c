/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bhsm.h"
#include "bhsm_priv.h"
#include "bsp_types.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_bsp_debug.h"
#include "bsp_headers.h"
#include "bsp_components.h"
#include "bsp_keyslot.h"

BDBG_MODULE( BHSMb );

#define _DUMP_RAW_COMMAND (0) /* set to 1 to print *raw* commands along with parsed format. */

#define _OFFSET_TO_INPUT_DATA (sizeof(Bsp_CmdHeader_InFields_t)/sizeof(uint32_t))
#define _OFFSET_TO_OUTPUT_DATA (sizeof(Bsp_CmdHeader_OutFields_t)/sizeof(uint32_t))

typedef struct {
    bool configured;
    bool io;
    unsigned byteOffset;
    unsigned size;       /* size in bytes */
    bool isArray;
    bool isEnum;
    unsigned enumIndex;  /* valid of isEnum is true. */
    char *pName;         /* parameter name. */
}_Parameter;

typedef struct
{
    char *pCommandName;

    _Parameter *pParametersInput;
    _Parameter *pParametersOutput;
    unsigned numParametersInput;
    unsigned numParametersOutput;

}_Command;

typedef struct
{
    char *pComponentName;
    unsigned conmponentIndex;
    unsigned maxCommandIndex;

    _Command *pCommands;   /* pointer to commands */

}_Component;

typedef struct
{
    unsigned enumIndex;            /* a inventory index for the enumerator */
    unsigned elementIndex;        /* the value of the enumerator element. */
    char *pName;                  /* element name */
    unsigned value;

}_EnumElement;

typedef struct
{
    char *pName;                    /* enumerator name */
    unsigned enumIndex;              /* a inventory index for the enumerator. it will range from 0 to maxElementIndex-1 */
    unsigned maxElementIndex;
    _EnumElement *pElements;   /* pointer to commands */
    bool configured;

}_Enum;

typedef struct BHSM_BspDebugModule
{
    BHSM_Handle hHsm;

    _Component *pComponents; /* pointer to components. */
    unsigned numComponents;

    _Enum   *pEnums;         /* pointer to enumerators.  */
    unsigned numEnums;

    bool componentsPrintFilter[Bsp_CmdComponent_eMax]; /* which componets to filter.*/
}BHSM_BspDebugModule;

static _Component* _GetComponent( BHSM_Handle hHsm, unsigned componentIndex );
static _Command* _GetCommand( _Component *pComponent, unsigned commandIndex );
static _Enum* _GetEnum( BHSM_Handle hHsm, unsigned enumIndex );
static _EnumElement* _GetElement( _Enum *pEnum, unsigned value );
static void _PrintParameter( BHSM_Handle hHsm, _Parameter *pParameter, const uint32_t *pData );
static void _PrintParameterOwnership( BHSM_Handle hHsm, const uint32_t *pData );
static bool _PrintFilter( BHSM_Handle hHsm, unsigned componentIndex );




BERR_Code BHSM_BspDebug_Init( BHSM_Handle hHsm, BHSM_BspDebugModuleSettings *pSettings )
{
    BHSM_BspDebugModule *pModule = NULL;
    BERR_Code rc = BERR_UNKNOWN;

    BSTD_UNUSED( pSettings );
    BDBG_ENTER( BHSM_BspDebug_Init );

    if( hHsm->modules.pBspDebug ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); } /* all ready initialised!*/

    pModule = BKNI_Malloc( sizeof(*pModule) );
    if( !pModule ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( pModule, 0, sizeof(*pModule) );

    hHsm->modules.pBspDebug = pModule;

    rc = BHSM_BspDebug_LoadAutoGeneratedData( hHsm );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(BERR_INVALID_PARAMETER); goto error; }

    /* specify which components to filter. Edit manually  */
    pModule->componentsPrintFilter[Bsp_CmdComponent_eRv]              = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eOtpMsp]          = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eControl]         = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eKeySlot]         = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eKeyLadder]       = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eOtpMisc]         = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eOtpMode0]        = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eOtpDataSection]  = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eMemcArch]        = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eGisbBlocker]     = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eBiuChecker]      = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eCrypto]          = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eEtsi5]           = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eHdcp1x]          = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eHdcp22]          = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eRpmb]            = true;
    pModule->componentsPrintFilter[Bsp_CmdComponent_eHwkl]            = true;

    BDBG_LEAVE( BHSM_BspDebug_Init );
    return BERR_SUCCESS;

error:
    if( pModule )  { BKNI_Free( pModule ); }
    hHsm->modules.pBspDebug = NULL;

    BDBG_LEAVE( BHSM_BspDebug_Init );
    return rc;
}


BERR_Code BHSM_BspDebug_Uninit( BHSM_Handle hHsm )
{
    BHSM_BspDebugModule *pModule = hHsm->modules.pBspDebug;
    unsigned componentIndex = 0;
    unsigned commandIndex = 0;
    unsigned enumIndex = 0;

    BDBG_ENTER( BHSM_BspDebug_Uninit );

    /* clean up */
    if( pModule ) {

        /* delete component database. */
        for( componentIndex = 0; componentIndex < pModule->numComponents; componentIndex++ ) {

            _Component *pComp = &pModule->pComponents[componentIndex];

            for( commandIndex = 0; commandIndex < pComp->maxCommandIndex; commandIndex++ ) {

                _Command *pCommand = &pComp->pCommands[commandIndex];

                if( pCommand->pParametersInput )BKNI_Free( pCommand->pParametersInput );
                if( pCommand->pParametersOutput )BKNI_Free( pCommand->pParametersOutput );
            }
            if( pComp->pCommands ) { BKNI_Free( pComp->pCommands ); }
        }
        if( pModule->pComponents ) { BKNI_Free( pModule->pComponents ); }

        /* delete enumerator database. */
        for( enumIndex = 0; enumIndex < pModule->numEnums; enumIndex++ ) {
            _Enum *pEnum = &pModule->pEnums[enumIndex];
            if( pEnum->pElements ) { BKNI_Free( pEnum->pElements ); }
        }
        if( pModule->pEnums ) { BKNI_Free( pModule->pEnums ); }

        /* delete the module data  */
        BKNI_Free( pModule );
        hHsm->modules.pBspDebug = NULL;
    }

    BDBG_LEAVE( BHSM_BspDebug_Uninit );
    return BERR_SUCCESS;
}


/* Print either an Input or output command. */
void BHSM_BspDebug_PrintCommand( BHSM_Handle hHsm, const uint32_t *pData )
{
    unsigned componentIndex = 0;
    unsigned commandIndex = 0;
    unsigned parameterIndex = 0;
    _Component *pComponent = NULL;
    _Command *pCommand = NULL;
    _Parameter *pParameter = NULL;

    BSTD_UNUSED( hHsm );
    BDBG_ENTER( BHSM_BspDebug_PrintResponse );
    if( !pData ){ BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    componentIndex = (pData[5] >> 8) & 0xFF;
    commandIndex   =  pData[5] & 0xFF;

    if( !_PrintFilter( hHsm, componentIndex ) ) { return; }

    pComponent = _GetComponent( hHsm, componentIndex );
    if( !pComponent ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    pCommand = _GetCommand( pComponent, commandIndex );
    if( !pCommand ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    BDBG_LOG(("\n[%s  %s]", pComponent->pComponentName, pCommand->pCommandName ));

    #if _DUMP_RAW_COMMAND
    {
        unsigned xx = 0;
        unsigned maxOffset = 0;
        for( xx = 0; xx < 384/4; xx++ ) /* TODO make BHSM_P_MAILBOX_BYTE_SIZE accessiable and use it */
        {
            if( pData[xx] ) { maxOffset = xx; }
        }

        for( xx = 0; xx <= maxOffset; xx++ )
        {
            BDBG_LOG(("> %02d 0x%08X", xx, pData[xx] ));
        }
    }
    #endif

    for( parameterIndex = 0; parameterIndex < pCommand->numParametersInput; parameterIndex++ )
    {
        pParameter = &pCommand->pParametersInput[parameterIndex];
        _PrintParameter( hHsm, pParameter, pData + _OFFSET_TO_INPUT_DATA );
    }

    BDBG_LEAVE( BHSM_BspDebug_PrintResponse );
    return;
}

void BHSM_BspDebug_PrintResponse( BHSM_Handle hHsm, const uint32_t *pData )
{
    unsigned componentIndex = 0;
    unsigned commandIndex = 0;
    unsigned parameterIndex = 0;
    _Component *pComponent = NULL;
    _Command *pCommand = NULL;
    _Parameter *pParameter = NULL;

    BSTD_UNUSED( hHsm );
    BDBG_ENTER( BHSM_BspDebug_PrintResponse );
    if( !pData ){ BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    componentIndex = (pData[3] >> 8) & 0xFF;
    commandIndex   =  pData[3] & 0xFF;

    if( !_PrintFilter( hHsm, componentIndex ) ) { return; }

    #if _DUMP_RAW_COMMAND
    {
        unsigned xx = 0;
        unsigned maxOffset = 0;
        for( xx = 0; xx < 384/4; xx++ ) /* TODO make BHSM_P_MAILBOX_BYTE_SIZE accessiable and use it */
        {
            if( pData[xx] ) { maxOffset = xx; }
        }

        for( xx = 0; xx <= maxOffset; xx++ )
        {
            BDBG_LOG(("> %02d 0x%08X", xx, pData[xx] ));
        }
    }
    #endif

    pComponent = _GetComponent( hHsm, componentIndex );
    if( !pComponent ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    pCommand = _GetCommand( pComponent, commandIndex );
    if( !pCommand ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }


    /*special case. */
    if( (componentIndex == Bsp_CmdComponent_eKeySlot) && (commandIndex == Bsp_CmdKeySlot_eQuery) )
    {

        _PrintParameterOwnership( hHsm, pData + _OFFSET_TO_OUTPUT_DATA );
    }
    else{

        for( parameterIndex = 0; parameterIndex < pCommand->numParametersOutput; parameterIndex++ )
        {
            pParameter = &pCommand->pParametersOutput[parameterIndex];
            _PrintParameter( hHsm, pParameter, pData + _OFFSET_TO_OUTPUT_DATA );
        }
    }


    BDBG_LEAVE( BHSM_BspDebug_PrintResponse );
    return;
}


BERR_Code BHSM_BspDebug_RegisterInit( BHSM_Handle hHsm, BHSM_BspDebugRegisterInit *pParam )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_BspDebugModule *pModule = NULL;

    BSTD_UNUSED( hHsm );
    BDBG_ENTER( BHSM_BspDebug_RegisterInit );
    if( !pParam ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }

    if( pModule->pComponents ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( pModule->pEnums ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    /* allocate space for components database */
    if( pParam->numComponents )
    {
        pModule->pComponents = BKNI_Malloc( sizeof(_Component) * pParam->numComponents );
        if( !pModule->pComponents ){ rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto error; }
        BKNI_Memset( pModule->pComponents, 0, sizeof(_Component) * pParam->numComponents );
    }
    pModule->numComponents = pParam->numComponents;

    /* allocate space for enums database */
    if( pParam->numEnums )
    {
        pModule->pEnums = BKNI_Malloc( sizeof(_Enum) * pParam->numEnums );
        if( !pModule->pEnums ){ rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto error; }
        BKNI_Memset( pModule->pEnums, 0, sizeof(_Enum) * pParam->numEnums );
    }
    pModule->numEnums = pParam->numEnums;

    BDBG_LEAVE( BHSM_BspDebug_RegisterInit );
    return BERR_SUCCESS;

error:
    if( pModule->pComponents ) BKNI_Free( pModule->pComponents );
    if( pModule->pEnums ) BKNI_Free( pModule->pEnums );

    BDBG_LEAVE( BHSM_BspDebug_RegisterInit );
    return rc;
}


BERR_Code BHSM_BspDebug_RegisterComponent( BHSM_Handle hHsm, BHSM_BspDebugRegisterComponent *pParam )
{
    BHSM_BspDebugModule *pModule = NULL;
    _Component *pComponent = NULL;

    BDBG_ENTER( BHSM_BspDebug_RegisterComponent );
    if( !pParam ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    /*BDBG_MSG(( "COMPONENT [%s]", pParam->pComponentName ));*/

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }
    if( !pModule->pComponents ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }
    if( pParam->componentIndex >= pModule->numComponents ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }

    pComponent = &pModule->pComponents[pParam->componentIndex];

    if( pComponent->pCommands ) { return BERR_TRACE(BERR_NOT_INITIALIZED); } /* component already initialised */

    if( pParam->maxCommandIndex )
    {
        pComponent->pCommands = BKNI_Malloc( sizeof(_Command) * pParam->maxCommandIndex );
        if( !pComponent->pCommands ){ return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); }
        BKNI_Memset( pComponent->pCommands, 0, sizeof(_Command) * pParam->maxCommandIndex );
    }

    pComponent->maxCommandIndex = pParam->maxCommandIndex;
    pComponent->pComponentName = pParam->pComponentName;

    BDBG_LEAVE( BHSM_BspDebug_RegisterComponent );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspDebug_RegisterCommand( BHSM_Handle hHsm, BHSM_BspDebugRegisterCommand *pParam )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_BspDebugModule *pModule = NULL;
    _Component *pComponent = NULL;
    _Command *pCommand = NULL;

    BDBG_ENTER( BHSM_BspDebug_RegisterCommand );
    if( !pParam ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    /* BDBG_MSG(( "COMMAND [%s]", pParam->pCommandName ));*/

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }
    if( !pModule->pComponents ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }
    if( pParam->componentIndex >= pModule->numComponents ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }

    pComponent = &pModule->pComponents[pParam->componentIndex];
    if( !pComponent->pCommands ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }
    if( pParam->commandIndex >= pComponent->maxCommandIndex ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }

    pCommand = &pComponent->pCommands[pParam->commandIndex];

    if( pCommand->pParametersInput ) { return BERR_TRACE(BERR_INVALID_PARAMETER); } /* already initialised */
    if( pCommand->pParametersOutput ) { return BERR_TRACE(BERR_INVALID_PARAMETER); } /* already initialised */

    if( pParam->numParametersInput )
    {
        pCommand->pParametersInput = BKNI_Malloc( sizeof(_Parameter) * pParam->numParametersInput );
        if( !pCommand->pParametersInput ) { rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto error; }

        BKNI_Memset( pCommand->pParametersInput, 0, sizeof(_Parameter) * pParam->numParametersInput );

        pCommand->numParametersInput = pParam->numParametersInput;
    }

    if( pParam->numParametersOutput )
    {
        pCommand->pParametersOutput = BKNI_Malloc( sizeof(_Parameter) * pParam->numParametersOutput );
        if( !pCommand->pParametersOutput ) { rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto error; }

        BKNI_Memset( pCommand->pParametersOutput, 0, sizeof(_Parameter) * pParam->numParametersOutput );

        pCommand->numParametersOutput = pParam->numParametersOutput;
    }
    pCommand->pCommandName = pParam->pCommandName;


    BDBG_LEAVE( BHSM_BspDebug_RegisterCommand );
    return BERR_SUCCESS;

error:
    if( pCommand->pParametersInput ) BKNI_Free( pCommand->pParametersInput );
    if( pCommand->pParametersOutput ) BKNI_Free( pCommand->pParametersOutput );

    return rc;
}


BERR_Code BHSM_BspDebug_RegisterParamter( BHSM_Handle hHsm, BHSM_BspDebugRegisterParamter *pParam )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspDebugModule *pModule = NULL;
    _Component *pComponent = NULL;
    _Command *pCommand = NULL;
    _Parameter *pParameter = NULL;

    BDBG_ENTER( BHSM_BspDebug_RegisterParamter );
    if( !pParam ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    /* BDBG_MSG(( "PARAMETER %s [%s]", pParam->io==BHSM_BSP_DEBUG_INPUT?" IN":"OUT",  pParam->pName )); */

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }
    if( !pModule->pComponents ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }

    if( pParam->componentIndex >= pModule->numComponents ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }
    pComponent = &pModule->pComponents[pParam->componentIndex];
    if( !pComponent->pCommands ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }

    if( pParam->commandIndex >= pComponent->maxCommandIndex ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }
    pCommand = &pComponent->pCommands[pParam->commandIndex];

    /* locate the parameter. */
    if( pParam->io == BHSM_BSP_DEBUG_INPUT )
    {
        if( !pCommand->pParametersInput ) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error; }
        if( pParam->parameterIndex >= pCommand->numParametersInput ) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error; }
        pParameter = &pCommand->pParametersInput[pParam->parameterIndex];
    }
    else if ( pParam->io == BHSM_BSP_DEBUG_OUTPUT )
    {
        if( !pCommand->pParametersOutput ) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error; }
        if( pParam->parameterIndex >= pCommand->numParametersOutput ) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error; }
        pParameter = &pCommand->pParametersOutput[pParam->parameterIndex];
    }
    else
    {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error;
    }

    if( pParameter->configured ) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error; }

    pParameter->configured = true;
    pParameter->io         = pParam->io;         /* is it input or output */
    pParameter->byteOffset = pParam->byteOffset;
    pParameter->size       = pParam->size;       /* size in bytes */
    pParameter->isArray    = pParam->isArray;
    pParameter->isEnum     = pParam->isEnum;
    pParameter->enumIndex  = pParam->enumIndex;  /* valid if isEnum is true */
    pParameter->pName      = pParam->pName;
error:

    BDBG_LEAVE( BHSM_BspDebug_RegisterParamter );
    return rc;
}

BERR_Code BHSM_BspDebug_RegisterEnum( BHSM_Handle hHsm, BHSM_BspDebugRegisterEnum *pParam )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspDebugModule *pModule = NULL;
    _Enum *pEnum = NULL;

    BDBG_ENTER( BHSM_BspDebug_RegisterEnum );
    if( !pParam ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    /*BDBG_MSG(( "ENUM [%s]", pParam->pName ));*/

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }
    if( !pModule->pEnums ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }

    if( pParam->enumIndex >= pModule->numEnums ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }

    pEnum = &pModule->pEnums[pParam->enumIndex];

    if( pEnum->configured ) { return BERR_TRACE(BERR_NOT_INITIALIZED); } /* already registered !!*/

    if( pParam->maxElementIndex )
    {
        pEnum->pElements = BKNI_Malloc( sizeof(_EnumElement) * pParam->maxElementIndex );
        if( !pEnum->pElements ){ rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto error; }
        BKNI_Memset( pEnum->pElements, 0, sizeof(_EnumElement) * pParam->maxElementIndex );
    }

    pEnum->pName = pParam->pName;
    pEnum->enumIndex = pParam->enumIndex;
    pEnum->maxElementIndex = pParam->maxElementIndex;
    pEnum->configured = true;

    BDBG_LEAVE( BHSM_BspDebug_RegisterEnum );
    return BERR_SUCCESS;
error:

    if( pEnum->pElements ) { BKNI_Free( pEnum->pElements ); }

    return rc;
}


BERR_Code BHSM_BspDebug_RegisterEnumElement( BHSM_Handle hHsm, BHSM_BspDebugRegisterEnumElement *pParam )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspDebugModule *pModule = NULL;
    _EnumElement *pElement = NULL;
    _Enum *pEnum = NULL;

    BDBG_ENTER( BHSM_BspDebug_RegisterEnumElement );
    if( !pParam ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    /*BDBG_MSG(( "ELEMENT [%s]", pParam->pName ));*/

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }
    if( !pModule->pEnums ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }

    if( pParam->enumIndex >= pModule->numEnums ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }
    pEnum = &pModule->pEnums[pParam->enumIndex];

    if( !pEnum->configured ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }

    if( pParam->elementIndex >= pEnum->maxElementIndex ){ return BERR_TRACE(BERR_NOT_INITIALIZED); }

    pElement = &pEnum->pElements[pParam->elementIndex];

    pElement->pName = pParam->pName;
    pElement->enumIndex = pParam->enumIndex;
    pElement->elementIndex = pParam->elementIndex;
    pElement->value = pParam->value;

    BDBG_LEAVE( BHSM_BspDebug_RegisterEnumElement );
    return rc;
}





static _Component* _GetComponent( BHSM_Handle hHsm, unsigned componentIndex )
{
    BHSM_BspDebugModule *pModule = NULL;

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ BERR_TRACE(BERR_NOT_INITIALIZED); return NULL; }
    if( !pModule->pComponents ) { BERR_TRACE(BERR_NOT_INITIALIZED); return NULL; }
    if( componentIndex >= pModule->numComponents ){ BERR_TRACE(BERR_NOT_INITIALIZED); return NULL; }

    return  &pModule->pComponents[componentIndex];
}

static _Command* _GetCommand( _Component *pComponent, unsigned commandIndex )
{
    if( !pComponent ) { BERR_TRACE(BERR_NOT_INITIALIZED); return NULL; }
    if( commandIndex >= pComponent->maxCommandIndex ) {
        BERR_TRACE(BERR_NOT_INITIALIZED);
        BDBG_ERR(("[%s] [%d][%d]", BSTD_FUNCTION, commandIndex, pComponent->maxCommandIndex ));
        return NULL;
    }

    return &pComponent->pCommands[commandIndex];
}


unsigned  _GetUnsigned( const uint32_t *pData, unsigned byteOffset, unsigned size )
{
    unsigned rv = 0;
    unsigned wordOffset = byteOffset/4;
    unsigned wordByteOffset = byteOffset % 4;

    switch( size )
    {
        case 1: {
            rv =  (pData[wordOffset] >> (8*(wordByteOffset))) & 0xFF;
            break;
        }
        case 2: {
            if( wordByteOffset >= 3 ) { BERR_TRACE(BERR_NOT_INITIALIZED); rv = 0; break; } /*can't wrap!*/

            rv =   ((pData[wordOffset] >> (8*(  wordByteOffset)) ) & 0xFF);
            rv += (((pData[wordOffset] >> (8*(++wordByteOffset)) ) & 0xFF) << 8);
            break;
        }
        case 3: {
            if( wordByteOffset >= 2 ) { BERR_TRACE(BERR_NOT_INITIALIZED); rv = 0; break; } /*can't wrap!*/

            rv =   ((pData[wordOffset] >> (8*(  wordByteOffset)) ) & 0xFF);
            rv += (((pData[wordOffset] >> (8*(++wordByteOffset)) ) & 0xFF) << 8);
            rv += (((pData[wordOffset] >> (8*(++wordByteOffset)) ) & 0xFF) << 16);
            break;
        }
        case 4: {
            rv = pData[wordOffset];
            break;
        }
        default: { BERR_TRACE(BERR_NOT_INITIALIZED); break; }
    }

    return rv;
}

static void _PrintParameterOwnership( BHSM_Handle hHsm, const uint32_t *pData )
{
    unsigned keySlotNumberIvPerSlot128 = 0;
    unsigned keySlotNumberIvPerBlock128 = 0;
    unsigned keySlotNumberIvPerBlock256 = 0;
    unsigned keySlotNumberIvPerEntry256 = 0;
    unsigned x;
    unsigned status;
    unsigned offset;

     BSTD_UNUSED( hHsm );

    if( !pData ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    keySlotNumberIvPerSlot128  = _GetUnsigned( pData, 0, 1 );
    keySlotNumberIvPerBlock128 = _GetUnsigned( pData, 1, 1 );
    keySlotNumberIvPerBlock256 = _GetUnsigned( pData, 2, 1 );
    keySlotNumberIvPerEntry256 = _GetUnsigned( pData, 3, 1 );

    offset = 4;

    for( x = 0; x < keySlotNumberIvPerSlot128; x++, offset++ ){

        status = _GetUnsigned( pData, offset, 1 );

        if( status ){
            BDBG_LOG(("  IvPerSlot128 #[%d] [%s]", x, status==1?"SAGE":"Shared" ));
        }
    }

    for( x = 0; x < keySlotNumberIvPerBlock128; x++, offset++ ){

        status = _GetUnsigned( pData, offset, 1 );

        if( status ){
            BDBG_LOG(("  IvPerBlock128 #[%d] [%s]", x, status==1?"SAGE":"Shared" ));
        }
    }

    for( x = 0; x < keySlotNumberIvPerBlock256; x++, offset++ ){

        status = _GetUnsigned( pData, offset, 1 );

        if( status ){
            BDBG_LOG(("  IvPerBlock256 #[%d] [%s]", x, status==1?"SAGE":"Shared" ));
        }
    }

    for( x = 0; x < keySlotNumberIvPerEntry256; x++, offset++ ){

        status = _GetUnsigned( pData, offset, 1 );

        if( status ){
            BDBG_LOG(("  IvPerEntry256 #[%d] [%s]", x, status==1?"SAGE":"Shared" ));
        }
    }

    return;
}


static void _PrintParameter( BHSM_Handle hHsm, _Parameter *pParameter, const uint32_t *pData )
{

    if( BKNI_Memcmp( "reserved", pParameter->pName, 8 ) == 0 ){
        /* don't print reserved parameters.*/
        return;
    }

    if( pParameter->isArray ){

        BDBG_LOG(("    %s %s", pParameter->io == BHSM_BSP_DEBUG_INPUT?">":"<", pParameter->pName  ));

        if( pParameter->byteOffset % 4 == 0 )
        {
            unsigned x;

            for( x = 0; x < pParameter->size; x+=4 ) {
                BDBG_LOG(("         %08x ",  _GetUnsigned( pData, pParameter->byteOffset+x, 4 ) ));
            }
        }
        else {
               BDBG_ERR(("Issue parsing array: [%s] size[%d]", pParameter->pName, pParameter->size ));
        }
    }
    else if( pParameter->isEnum ) {
        _Enum *pEnum = NULL;
        _EnumElement *pElement = NULL;
        char valueStr[32] = {0};
        unsigned value;

        pEnum = _GetEnum( hHsm, pParameter->enumIndex );
        value = _GetUnsigned( pData, pParameter->byteOffset, pParameter->size );
        pElement = _GetElement( pEnum, value );

        if( pElement && pElement->pName) {
            BKNI_Snprintf(  valueStr, sizeof(valueStr), "%s", pElement->pName );
        }
        else{
            BKNI_Snprintf(  valueStr, sizeof(valueStr), "enum-0x%x", value );
        }

        BDBG_LOG(("    %s %s [%s]", pParameter->io == BHSM_BSP_DEBUG_INPUT?">":"<"
                                  , pParameter->pName
                                  , valueStr ));
    }
    else{
        unsigned value;
        value = _GetUnsigned( pData, pParameter->byteOffset, pParameter->size );

        BDBG_LOG(("    %s %s [0x%x]", pParameter->io == BHSM_BSP_DEBUG_INPUT?">":"<"
                                  , pParameter->pName
                                  , value ));
    }

    return;
}


static _Enum* _GetEnum( BHSM_Handle hHsm, unsigned enumIndex )
{
    BHSM_BspDebugModule *pModule = NULL;

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ BERR_TRACE(BERR_NOT_INITIALIZED); return NULL; }
    if( !pModule->pEnums ) { BERR_TRACE(BERR_NOT_INITIALIZED); return NULL; }

    if( enumIndex >= pModule->numEnums ) { BERR_TRACE(BERR_NOT_INITIALIZED); return NULL; }

    return &pModule->pEnums[enumIndex];
}


static _EnumElement* _GetElement( _Enum *pEnum, unsigned value )
{
    unsigned i = 0;

    if( !pEnum ) { BERR_TRACE(BERR_NOT_INITIALIZED); return NULL; }
    if( !pEnum->pElements ) { return NULL; }

    for( i = 0; i < pEnum->maxElementIndex; i++ ){

        _EnumElement *pElement = NULL;

        pElement = &pEnum->pElements[i];

        if( value == pElement->value ) return pElement;
   }

    BDBG_ERR(("Invalid element value [%u] for type [%s]", value, pEnum->pName?pEnum->pName:"unknow" ));
    return NULL;
}

static bool _PrintFilter( BHSM_Handle hHsm, unsigned componentIndex )
{
    BHSM_BspDebugModule *pModule = NULL;

    pModule = hHsm->modules.pBspDebug;
    if( !pModule ){ BERR_TRACE(BERR_NOT_INITIALIZED);  return false; }
    if( componentIndex >= Bsp_CmdComponent_eMax ) { BERR_TRACE(BERR_NOT_INITIALIZED); return false; }

    return pModule->componentsPrintFilter[componentIndex];
}
