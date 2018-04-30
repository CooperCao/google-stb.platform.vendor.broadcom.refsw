/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bstd.h"
#include "bhsm.h"
#include "bchp_bsp_cmdbuf.h"
#include "bsp_s_hw.h"
#include "bchp_bsp_control_intr2.h"
#include "bchp_bsp_glb_control.h"
#include "bsp_s_commands.h"
#include "bhsm_bsp_msg.h"
#include "bchp_int_id_bsp.h"
#include "bhsm_priv.h"
#include "bsp_s_keycommon.h"

/* special support for  HSM ISR  vs. HSM Polling  */
#if BHSM_DEBUG_POLLING
#include "bchp_timer.h"
#endif

#if BHSM_DEBUG_INDIVIDUAL_COMMAND
#include <stdlib.h>
#endif

BDBG_MODULE(BHSMa);

#ifndef BHSM_SECURE_MEMORY_SIZE
#define BHSM_SECURE_MEMORY_SIZE       (4)
#endif
#define BHSM_BSP_TIMEOUT_MS   2000

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define MAILBOX_IREADY                            BCHP_BSP_GLB_CONTROL_GLB_IRDY
#ifdef BHSM_BUILD_HSM_FOR_SAGE
    /* SAGE Context */
    #define MAILBOX_OLOAD_INTERRUPT_ID            BCHP_INT_ID_BSP_OLOAD1_INTR
    #define MAILBOX_OLOAD_INTERRUPT_STATUS_MASK   BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOAD1_INT_MASK
    #define MAILBOX_OLOAD_CONTROL                 BCHP_BSP_GLB_CONTROL_GLB_OLOAD1
    #define MAILBOX_OLOAD_CONTROL_MASK            BCHP_BSP_GLB_CONTROL_GLB_OLOAD1_CMD_OLOAD1_MASK
    #define MAILBOX_ILOAD_CONTROL                 BCHP_BSP_GLB_CONTROL_GLB_ILOAD1
    #define MAILBOX_ILOAD_CONTROL_MASK            BCHP_BSP_GLB_CONTROL_GLB_ILOAD1_CMD_ILOAD1_MASK
    #define MAILBOX_ADDRESS_IN                    BHSM_IN_BUF1_ADDR
    #define MAILBOX_ADDRESS_OUT                   BHSM_OUT_BUF1_ADDR
    #define MAILBOX_IREADY_MASK                   BCHP_BSP_GLB_CONTROL_GLB_IRDY_CMD_IDRY1_MASK
#else
    /* HOST Context */
    #define MAILBOX_OLOAD_INTERRUPT_ID            BCHP_INT_ID_BSP_OLOAD2_INTR
    #define MAILBOX_OLOAD_INTERRUPT_STATUS_MASK   BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOAD2_INT_MASK
    #define MAILBOX_OLOAD_CONTROL                 BCHP_BSP_GLB_CONTROL_GLB_OLOAD2
    #define MAILBOX_OLOAD_CONTROL_MASK            BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK
    #define MAILBOX_ILOAD_CONTROL                 BCHP_BSP_GLB_CONTROL_GLB_ILOAD2
    #define MAILBOX_ILOAD_CONTROL_MASK            BCHP_BSP_GLB_CONTROL_GLB_ILOAD2_CMD_ILOAD2_MASK
    #define MAILBOX_ADDRESS_IN                    BHSM_IN_BUF2_ADDR
    #define MAILBOX_ADDRESS_OUT                   BHSM_OUT_BUF2_ADDR
    #define MAILBOX_IREADY_MASK                   BCHP_BSP_GLB_CONTROL_GLB_IRDY_CMD_IDRY2_MASK
#endif


static void mailboxInterruptHandler_isr( void* pHsm, int unused );
static char* cropStr( char* pStr );
static void  _ParseBfwVersion( BHSM_BfwVersion *pVersion, uint8_t epochScheme, uint32_t version );


BDBG_OBJECT_ID(BHSM_P_BspMsg);

/* BSP Message Module data. */
typedef struct BHSM_P_BspMsg_Module_s
{
    bool bspInterfaceBusy;             /* Indicates that HSM is communicating with BSP interface. Used for debug */
    BCMD_cmdType_e currentBspCommand;  /* Current/Last Bsp command executed. */

    unsigned sequenceNumber;          /*incremented by 1 for each command sent to the bsp (from this interface.)*/

    BREG_Handle    hRegister;
    BHSM_BspMsgConfigure_t conf;

    uint32_t *pSecureWord;            /* Pointer to memory location (the size of a uint32_t) that is secure. */
    uint32_t insecureWord;            /* When there is no secure memory loaction availabe/required, pSecureWord will point to this value. */

    BKNI_EventHandle   oLoadWait;
    BINT_CallbackHandle  IntCallback;

}BHSM_P_BspMsg_Module_t;


typedef struct BHSM_P_BspMsg
{
    BDBG_OBJECT( BHSM_P_BspMsg )

    BHSM_Handle    hHsm;
    BREG_Handle    hRegister;
    unsigned       cmdLength;                           /* The command length                            */
    uint16_t       responceLength;
    BCMD_cmdType_e bspCommand;                          /* The command identifier  */

    unsigned       packErrorCount;                      /* errors detected while packing a message will resulting a fail on submit */
    bool           verbose;                             /* allow disabling of debug on very frequent calls */
    bool           isRaw;                               /* When True, the command is assumed to be raw, the client is responsible for composing the header. */

    BHSM_P_BspMsg_Module_t *pMod;                       /* pointer to module data, for convenience  */

    #if BHSM_DEBUG_INDIVIDUAL_COMMAND
    BDBG_Level     level;                               /* store debug level */
    #endif
}BHSM_P_BspMsg;


/* macros to read and write to the mailbox.  */
#define writeOutbox(pModuleData,wordOffset,value) BREG_Write32(((pModuleData)->hRegister), ( MAILBOX_ADDRESS_IN  +(wordOffset)*4), (value) )
#define readOutbox(pModuleData,wordOffset)        BREG_Read32(((pModuleData)->hRegister),  ( MAILBOX_ADDRESS_IN  +(wordOffset)*4) )
#define readInbox(pModuleData,wordOffset)         BREG_Read32(((pModuleData)->hRegister),  ( MAILBOX_ADDRESS_OUT +(wordOffset)*4) )

BERR_Code BHSM_BspMsg_Init( BHSM_Handle hHsm, BHSM_BspMsgConfigure_t *pConfig )
{
    BHSM_P_BspMsg_Module_t *pModuleData;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_BspMsg_Init );

    /* hHsm is valid, as its object is set after this call. */

    if( !pConfig ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !(BHSM_P_Handle*)hHsm->regHandle ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pModuleData = BKNI_Malloc( sizeof(*pModuleData) );
    if( pModuleData == NULL )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset( pModuleData, 0, sizeof(*pModuleData) );

    pModuleData->conf = *pConfig;
    pModuleData->hRegister = hHsm->regHandle;
    pModuleData->sequenceNumber = 42;

    hHsm->pBspMessageModule = pModuleData;

    BDBG_CASSERT( BHSM_SECURE_MEMORY_SIZE >= sizeof(uint32_t) );
    if( pConfig->secureMemory.p && pConfig->secureMemory.size >= sizeof(uint32_t) )
    {
        pModuleData->pSecureWord = pConfig->secureMemory.p;
    }
    else
    {
        pModuleData->pSecureWord = &pModuleData->insecureWord;
    }


    /* initialise mailbox interface registers. */
    BREG_Write32( hHsm->regHandle, MAILBOX_OLOAD_CONTROL, 0 );
    BREG_Write32( hHsm->regHandle, MAILBOX_ILOAD_CONTROL, 0 );

   #ifdef BHSM_BUILD_HSM_FOR_HOST
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN, 0);
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS, 0);
    #ifdef BCHP_BSP_GLB_CONTROL_GLB_RAAGA_INTR_STATUS /* define may not be available/relevant on legacy platforms */
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_RAAGA_INTR_STATUS, 0xFFFFFFFFL );
    #endif
   #endif

    rc = BINT_CreateCallback( &(pModuleData->IntCallback),
                                hHsm->interruptHandle,
                                MAILBOX_OLOAD_INTERRUPT_ID,
                                mailboxInterruptHandler_isr,
                                (void*)hHsm,
                                0x00 );
    if( rc != BERR_SUCCESS )
    {
        return  BERR_TRACE( rc );
    }

    if( ( rc = BINT_EnableCallback( pModuleData->IntCallback ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    if( ( rc = BKNI_CreateEvent( &(pModuleData->oLoadWait) ) ) != BERR_SUCCESS )
    {
        return  BERR_TRACE( rc );
    }

    BDBG_LEAVE( BHSM_BspMsg_Init );

    return BERR_SUCCESS;
}

void BHSM_BspMsg_Uninit( BHSM_Handle hHsm )
{

    BDBG_ENTER( BHSM_BspMsg_Uninit );

    if( hHsm->pBspMessageModule )
    {
        BHSM_P_BspMsg_Module_t *pModuleData = hHsm->pBspMessageModule;

        BKNI_DestroyEvent( pModuleData->oLoadWait );

        if( BINT_DisableCallback( pModuleData->IntCallback ) != BERR_SUCCESS )
        {
            (void)BERR_TRACE( BHSM_STATUS_FAILED );  /* continue, best effort */
        }

        if( BINT_DestroyCallback( pModuleData->IntCallback ) != BERR_SUCCESS )
        {
            (void)BERR_TRACE( BHSM_STATUS_FAILED );
        }

        BKNI_Free( hHsm->pBspMessageModule );
        hHsm->pBspMessageModule = NULL;
    }

    BDBG_LEAVE( BHSM_BspMsg_Uninit );
    return;
}


BERR_Code BHSM_BspMsg_Create( BHSM_Handle hHsm,  BHSM_BspMsg_h *phMsg )
{
    BHSM_BspMsg_h hMsg;

    BDBG_ENTER ( BHSM_BspMsg_Create );

    if( !hHsm )                    { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !hHsm->pBspMessageModule ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !phMsg )                   { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    *phMsg = NULL;

    hMsg = (BHSM_BspMsg_h) BKNI_Malloc( sizeof(BHSM_P_BspMsg) );
    if( hMsg == NULL)
    {
        BDBG_ERR(( "Failed to allocate mem for Mail [%d]", (unsigned)sizeof(BHSM_P_BspMsg) ));
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    BKNI_Memset( hMsg, 0, sizeof(BHSM_P_BspMsg) );

    hMsg->isRaw = true;
    hMsg->hHsm = hHsm;
    hMsg->pMod = (BHSM_P_BspMsg_Module_t*)hHsm->pBspMessageModule;

    BDBG_OBJECT_SET( hMsg, BHSM_P_BspMsg );
    *phMsg = hMsg;

    if( hMsg->pMod->bspInterfaceBusy )
    {
        BDBG_ERR(( "Only one BspMsg Message Object should be instantiated at any one time" ));
    }
    hMsg->pMod->bspInterfaceBusy = true;

    #if BHSM_DEBUG_INDIVIDUAL_COMMAND
    BDBG_GetModuleLevel( "BHSMa", &hMsg->level );
    #endif

    BDBG_LEAVE( BHSM_BspMsg_Create );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_Destroy ( BHSM_BspMsg_h hMsg )
{
    BDBG_ENTER( BHSM_BspMsg_Destroy );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    #if BHSM_DEBUG_INDIVIDUAL_COMMAND
    BDBG_SetModuleLevel( "BHSMa", hMsg->level );
    #endif

    hMsg->pMod->bspInterfaceBusy = false;

    BDBG_OBJECT_DESTROY( hMsg, BHSM_P_BspMsg );
    BKNI_Free( hMsg );

    BDBG_LEAVE( BHSM_BspMsg_Destroy );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_Pack8_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t data, char *pCommand)
{
    signed wordOffset;
    signed byteShift;

    BDBG_ENTER( BHSM_BspMsg_Pack8 );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    if( offset >= BHSM_P_BSP_MSG_SIZE )
    {
        hMsg->packErrorCount++;
        return BHSM_STATUS_INPUT_PARM_ERR;
    }

    wordOffset = offset/4;
    byteShift  = 3-(offset%4);

    *hMsg->pMod->pSecureWord = readOutbox( hMsg->pMod, wordOffset );
    *hMsg->pMod->pSecureWord |= (uint32_t)(data << (8*byteShift));
    writeOutbox( hMsg->pMod, wordOffset, *hMsg->pMod->pSecureWord );

    if( (offset+1) > hMsg->cmdLength )  /* record the greatest offset.  */
    {
        hMsg->cmdLength = offset+1;
    }

    if( hMsg->verbose )
    {
        BDBG_MSG(( ">[%03d-%d] [%02X] %s", offset/4, offset%4, data, (pCommand ? cropStr(pCommand):"") ));
    }

    BDBG_LEAVE( BHSM_BspMsg_Pack8 );
    return BERR_SUCCESS;
}



BERR_Code BHSM_BspMsg_Pack16_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint16_t data, char *pCommand )
{
    signed wordOffset;
    signed byteShift;

    BDBG_ENTER( BHSM_BspMsg_Pack16 );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    if( offset >= BHSM_P_BSP_MSG_SIZE )
    {
        hMsg->packErrorCount++;
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    wordOffset = offset/4;
    byteShift  = 2-(offset%4);

    if( byteShift < 0 )
    {
        hMsg->packErrorCount++;
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    data &= 0xFFFF;

    *hMsg->pMod->pSecureWord = readOutbox( hMsg->pMod, wordOffset );
    *hMsg->pMod->pSecureWord |= (uint32_t)(data << (8*byteShift));
    writeOutbox( hMsg->pMod, wordOffset, *hMsg->pMod->pSecureWord );

    if( (offset+2) > hMsg->cmdLength )  /* record the greatest offset.  */
    {
        hMsg->cmdLength = offset+2;
    }

    if( hMsg->verbose )
    {
        BDBG_MSG(( ">[%03d-%d] [%04X] %s", offset/4, offset%4, data, (pCommand ? cropStr(pCommand):"") ));
    }

    BDBG_LEAVE( BHSM_BspMsg_Pack16 );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_Pack24_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t data, char *pCommand )
{
    signed wordOffset;
    signed byteShift;

    BDBG_ENTER( BHSM_BspMsg_Pack24 );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    if( offset >= BHSM_P_BSP_MSG_SIZE )
    {
        hMsg->packErrorCount++;
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    wordOffset = offset/4;
    byteShift    = offset%4;

    if( byteShift > 1 )
    {
        hMsg->packErrorCount++;
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    byteShift = 1 - byteShift;
    data &= 0xFFFFFF;

    *hMsg->pMod->pSecureWord = readOutbox( hMsg->pMod, wordOffset );
    *hMsg->pMod->pSecureWord |= (uint32_t)(data << (8*byteShift));
    writeOutbox( hMsg->pMod, wordOffset, *hMsg->pMod->pSecureWord );

    if( (offset+3) > hMsg->cmdLength )  /* record the greatest offset.  */
    {
        hMsg->cmdLength = offset+3;
    }

    if( hMsg->verbose )
    {
        BDBG_MSG(( ">[%03d-%d] [%06X] %s", offset/4, offset%4, data, (pCommand ? cropStr(pCommand):"") ));
    }

    BDBG_LEAVE( BHSM_BspMsg_Pack24 );
    return BERR_SUCCESS;
}



BERR_Code BHSM_BspMsg_Pack32_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t data, char *pCommand )
{
    signed wordOffset;
    BDBG_ENTER( BHSM_BspMsg_Pack32 );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    if( offset >= BHSM_P_BSP_MSG_SIZE )
    {
        hMsg->packErrorCount++;
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    wordOffset = offset/4;

    if( offset%4 )
    {
        hMsg->packErrorCount++;
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    writeOutbox( hMsg->pMod, wordOffset, data );

    if( (offset+4) > hMsg->cmdLength )  /* record the greatest offset.  */
    {
        hMsg->cmdLength = offset+4;
    }

    if( hMsg->verbose )
    {
        BDBG_MSG(( ">[%03d-%d] [%08X] %s", offset/4, offset%4, data, (pCommand ? cropStr(pCommand):"") ));
    }

    BDBG_LEAVE( BHSM_BspMsg_Pack32 );
    return BERR_SUCCESS;
}



BERR_Code BHSM_BspMsg_PackArray_impl( BHSM_BspMsg_h hMsg, unsigned offset, const uint8_t *pData, unsigned length, char *pCommand )
{
    signed wordOffset;
    signed byteShift;
    signed index = 0;

    BDBG_ENTER( BHSM_BspMsg_PackArray );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );
    if( !pData ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( (offset + length) > BHSM_P_BSP_MSG_SIZE )
    {
        hMsg->packErrorCount++;
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    for( index = 0; index < (signed)length; index++ )
    {
        wordOffset = (offset+index)/4;
        byteShift  = 3-((offset+index)%4);

        *hMsg->pMod->pSecureWord = readOutbox( hMsg->pMod, wordOffset );
        *hMsg->pMod->pSecureWord |= (uint32_t)(pData[index] << (8*byteShift));
        writeOutbox( hMsg->pMod, wordOffset, *hMsg->pMod->pSecureWord );
    }

    if( (offset + length) > hMsg->cmdLength )  /* record the greatest offset.  */
    {
        hMsg->cmdLength = offset + length;
    }

    if( hMsg->verbose )
    {
        BDBG_MSG(( ">[%03d-%d] len[%03d] %s", offset/4, offset%4, length, (pCommand ? cropStr(pCommand):"") ));
        for( index = 0; index < (signed)length; index+=4 )
        {
            BDBG_MSG(( ">[%02X %02X %02X %02X]", pData[index], pData[index+1], pData[index+2], pData[index+3] ));
        }
    }

    BDBG_LEAVE( BHSM_BspMsg_PackArray );
    return BERR_SUCCESS;
}



BERR_Code BHSM_BspMsg_Get8_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t *pData, char* pCommand )
{
    BDBG_ENTER( BHSM_BspMsg_Get8 );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );
    if( !pData ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( (offset >= BHSM_P_BSP_MSG_SIZE) || (pData == NULL) )
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    *hMsg->pMod->pSecureWord = readInbox( hMsg->pMod, offset/4 );

    *pData = ( *hMsg->pMod->pSecureWord >> ((3-(offset%4))*8)) & 0xFF;

    if( hMsg->verbose )
    {
        BDBG_MSG(( "<[%03d-%d] [%02X]  %s", offset/4, offset%4, *pData, (pCommand ? cropStr(pCommand):"") ));
    }

    BDBG_LEAVE( BHSM_BspMsg_Get8 );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_Get16_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint16_t *pData, char* pCommand  )
{
    BDBG_ENTER( BHSM_BspMsg_Get16 );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );
    if( !pData ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( (offset+1 >= BHSM_P_BSP_MSG_SIZE) || (pData == NULL) ) /*1 byte after offset read */
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    *hMsg->pMod->pSecureWord = readInbox( hMsg->pMod, offset/4 );
    *pData =  *hMsg->pMod->pSecureWord >> ((2-(offset%4))*8);

    if( hMsg->verbose )
    {
        BDBG_MSG(( "<[%03d-%d] [%04X] %s", offset/4, offset%4, *pData, (pCommand ? cropStr(pCommand):"") ));
    }

    BDBG_LEAVE( BHSM_BspMsg_Get16 );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_Get24_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t *pData, char* pCommand  )
{
    BDBG_ENTER( BHSM_BspMsg_Get24 );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );
    if( !pData ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( (offset+2 >= BHSM_P_BSP_MSG_SIZE) || (pData == NULL) ) /* 2 bytes after offset read */
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    *hMsg->pMod->pSecureWord = readInbox( hMsg->pMod, offset/4 );
    *pData = *hMsg->pMod->pSecureWord >> ((1-(offset%4))*8);

    if( hMsg->verbose )
    {
        BDBG_MSG(( "<[%03d-%d] [%06X] %s", offset/4, offset%4, *pData, (pCommand ? cropStr(pCommand):"") ));
    }

    BDBG_LEAVE( BHSM_BspMsg_Get24 );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_Get32_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t *pData, char* pCommand )
{
    BDBG_ENTER( BHSM_BspMsg_Get32 );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );
    if( !pData ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( (offset+3 >= BHSM_P_BSP_MSG_SIZE) || (pData == NULL) ) /* 3 bytes after offset read */
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    *pData = readInbox( hMsg->pMod, offset/4 );

    if( hMsg->verbose )
    {
        BDBG_MSG(( "<[%03d-%d] [%08X] %s", offset/4, offset%4, *pData, (pCommand ? cropStr(pCommand):"") ));
    }

    BDBG_LEAVE( BHSM_BspMsg_Get32 );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_GetArray_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t  *pData, unsigned int length, char* pCommand )
{
    signed   index = 0;
    unsigned i;

    BDBG_ENTER( BHSM_BspMsg_GetArray );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );
    if( !pData ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( (offset+length >= BHSM_P_BSP_MSG_SIZE) || (pData == NULL) )
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    for( i = 0; i < length; i++ )
    {
        *hMsg->pMod->pSecureWord = readInbox( hMsg->pMod, (offset+i)/4 );
        pData[i] = (*hMsg->pMod->pSecureWord >> ((3-((offset+i)%4))*8)) & 0xFF;
    }

    if( hMsg->verbose )
    {
        BDBG_MSG(( "<[%03d-%d]%s", offset/4, offset%4, (pCommand ? cropStr(pCommand):"") ));
        for( index = 0; index < (signed)length; index+=4 )
        {
            BDBG_MSG(( "<[%02X %02X %02X %02X]", pData[index], pData[index+1], pData[index+2], pData[index+3] ));
        }
    }

    BDBG_LEAVE( BHSM_BspMsg_GetArray );
    return BERR_SUCCESS;
}

void BHSM_BspMsg_GetDefaultHeader( BHSM_BspMsgHeader_t *pHeader )
{
    BDBG_ENTER( BHSM_BspMsg_GetDefaultHeader );

    if( !pHeader )
    {
         BERR_TRACE( BERR_INVALID_PARAMETER );
         return;
    }

    BKNI_Memset( pHeader, 0, sizeof(BHSM_BspMsgHeader_t) );

    pHeader->verbose = true;

    BDBG_LEAVE( BHSM_BspMsg_GetDefaultHeader );
    return;
}



#define  BHSM_BSP_OFFSET_ABCDEF     (2<<2)
#define  BHSM_BSP_OFFSET_55AA       ((3<<2)+1)
#define  BHSM_BSP_OFFSET_eTagIdInv  ((3<<2)+0)
#define  BHSM_BSP_OFFSET_789A       (4<<2)

BERR_Code BHSM_BspMsg_Header_impl( BHSM_BspMsg_h hMsg, BCMD_cmdType_e bspCommand, BHSM_BspMsgHeader_t *pHeader, char *pCommand )
{
    unsigned i;
    uint32_t    iReady = 0;
    BDBG_ENTER( BHSM_BspMsg_Header );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );
    if( !pHeader  ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    hMsg->hRegister = hMsg->hHsm->regHandle;

    #if BHSM_DEBUG_INDIVIDUAL_COMMAND
    /* debug and individual command */
    {
        char *p = getenv("msg_hsm_command");
        unsigned command = 0;

        if( p )
        {
            command = atoi( p );
            if( command == (unsigned)bspCommand )
            {
                BDBG_SetModuleLevel( "BHSMa", BDBG_eMsg );
            }
        }
    }
    #endif

    /* Wait until BSP can accept input */
    for( i = 0; i < (BHSM_BSP_TIMEOUT_MS * 500); i++ )
    {
        iReady = BREG_Read32( hMsg->hRegister, MAILBOX_IREADY );
        if( iReady & MAILBOX_IREADY_MASK )
        {
            break;
        }
        BKNI_Delay( 2 );
    }

    if( i == (BHSM_BSP_TIMEOUT_MS * 500) )
    {
        hMsg->packErrorCount++;
        BDBG_ERR(( "TIMEOUT iREADY[%08X] command[0x%X]", iReady, bspCommand ));
        return BERR_TRACE( BHSM_STATUS_IRDY_ERR );
    }

    /* Clear the mailbox. */
    for( i = 0; i < BHSM_P_BSP_MSG_SIZE/sizeof(uint32_t); i++ )
    {
        writeOutbox( hMsg->pMod, i, 0 );
    }

    hMsg->packErrorCount = 0;
    hMsg->verbose    = pHeader->verbose;
    hMsg->isRaw      = pHeader->isRaw;
    hMsg->bspCommand = bspCommand;

    if( pHeader->commandLength > BHSM_P_BSP_MSG_SIZE )
    {
        hMsg->packErrorCount++;
        BDBG_ERR(( "Invalid parameter length [%X]", pHeader->commandLength ));
        return BHSM_STATUS_PARM_LEN_ERR;
    }

    /* Reset/set the instance data*/
    hMsg->responceLength = 0;
    hMsg->cmdLength  = pHeader->commandLength;

    if( pCommand ) {
        BDBG_MSG(( "COMMAND [%s]", (pCommand?pCommand:"") ));
    }
    else {
        BDBG_MSG(( "COMMAND [0x%02x]", bspCommand ));
    }

    if( hMsg->isRaw == false )
    {
        /* Compile Header */
        BHSM_BspMsg_Pack8 ( hMsg, BCMD_CommonBufferFields_eVerNum+3  , 0x10 );
        BHSM_BspMsg_Pack8 ( hMsg, BCMD_CommonBufferFields_eOwnerId   , hMsg->pMod->sequenceNumber );
        BHSM_BspMsg_Pack8 ( hMsg, BCMD_CommonBufferFields_eContMode  , pHeader->continualMode  );
        BHSM_BspMsg_Pack24( hMsg, BHSM_BSP_OFFSET_ABCDEF,              0xABCDEF );
        BHSM_BspMsg_Pack8 ( hMsg, BCMD_CommonBufferFields_eTagId     , bspCommand);
        BHSM_BspMsg_Pack16( hMsg, BHSM_BSP_OFFSET_55AA               , 0x55AA );
        BHSM_BspMsg_Pack8 ( hMsg, BHSM_BSP_OFFSET_eTagIdInv          , ~(bspCommand) );
        /* Set length until when submitting the command  */
        BHSM_BspMsg_Pack16( hMsg, BHSM_BSP_OFFSET_789A               , 0x789A );


        if( BCMD_CommonBufferFields_eParamStart > hMsg->cmdLength )
        {
            hMsg->cmdLength = BCMD_CommonBufferFields_eParamStart;
        }
    }

    BDBG_LEAVE( BHSM_BspMsg_Header );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_SubmitCommand( BHSM_BspMsg_h hMsg )
{

    BERR_Code rc = BERR_SUCCESS;
    BREG_Handle hRegister;
    BHSM_Handle hHsm;
    uint32_t modulus;
    #if BHSM_DEBUG_DUMP_RAW_BSP_COMMAND
    uint32_t i = 0;
    #endif

    BDBG_ENTER( BHSM_BspMsg_SubmitCommand );

    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    hRegister = hMsg->hRegister;
    hHsm = hMsg->hHsm;

    if( hMsg->packErrorCount )
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    if( (hMsg->cmdLength > BHSM_P_BSP_MSG_SIZE) || ( hMsg->cmdLength < BCMD_CommonBufferFields_eParamStart ) )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    modulus = hMsg->cmdLength % sizeof(uint32_t);
    if( modulus )
    {
        hMsg->cmdLength += (sizeof(uint32_t) - modulus);
    }

    if( hMsg->isRaw == false )
    {
        /* configure paramter length */
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
        BHSM_BspMsg_Pack16( hMsg, BCMD_CommonBufferFields_eParamLen, (hMsg->cmdLength - BCMD_CommonBufferFields_eParamStart)); /* (15:0) */
       #else
        /* set the command length to whole buffer length. */
        BHSM_BspMsg_Pack16( hMsg, BCMD_CommonBufferFields_eParamLen, (BHSM_P_BSP_MSG_SIZE - 20) ); /* buffer size - header size */
       #endif
    }

    #if BHSM_DEBUG_DUMP_RAW_BSP_COMMAND
    for( i = 0; i < hMsg->cmdLength; i += sizeof(uint32_t) )
    {
        uint32_t regVal;
        regVal = readOutbox( hMsg->pMod, i/4 );
        BDBG_LOG(("MSG> %.3d - %08X", (i/4), regVal ));
    }
    #endif

    BKNI_ResetEvent( hMsg->pMod->oLoadWait ); /* CTRL-C or any system level signal*/

    /* indicate that message is loaded. */
    BREG_Write32( hRegister, MAILBOX_ILOAD_CONTROL, MAILBOX_ILOAD_CONTROL_MASK );

    rc = BKNI_WaitForEvent( hMsg->pMod->oLoadWait, BHSM_BSP_TIMEOUT_MS );
    /* If event fails, retry by polling the registers. */
    if( rc != BERR_SUCCESS )
    {
        uint32_t registerValue = 0;
        unsigned timeout;

        for( timeout = 2000; timeout > 0; timeout-- )
        {
            BKNI_Sleep( 5 /*ms*/);

            registerValue = BREG_Read32( hRegister, MAILBOX_OLOAD_CONTROL );

            if( registerValue & MAILBOX_OLOAD_CONTROL_MASK )
            {
                break;
            }
        }

        if( timeout == 0  )
        {
            BDBG_ERR(("BHSM_BspMsg_SubmitCommand: TIMEOUT on command[0x%02X] GLB_OLOAD[0x%02X]", hMsg->bspCommand, registerValue ));
            return BERR_TRACE( BHSM_STATUS_TIME_OUT );
        }

        BDBG_WRN(("BKNI_WaitForEvent FAILED[%d] [%d] command[0x%02X]. Polled[%d]", rc, BHSM_BSP_TIMEOUT_MS, hMsg->bspCommand, timeout ));

        /* Reset the register. */
        BREG_Write32( hRegister, MAILBOX_OLOAD_CONTROL, registerValue & (~MAILBOX_OLOAD_CONTROL_MASK) );
    }

    BHSM_BspMsg_Get16( hMsg, BCMD_CommonBufferFields_eParamLen, &(hMsg->responceLength) );


    if( !hHsm->bfwVersionValid )
    {
        uint8_t epochScheme = 0;
        uint32_t version = 0;

       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
        BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eVersionScheme, &epochScheme );
       #endif
        BHSM_BspMsg_Get32( hMsg, BCMD_CommonBufferFields_eVerNum, &version );

        _ParseBfwVersion( &(hHsm->bfwVersion), epochScheme, version );
        hHsm->bfwVersionValid = true;
    }



    /* read the response header */
    #if BHSM_DEBUG_DUMP_RAW_BSP_COMMAND
    for( i = 0; i < (uint32_t)BCMD_DATA_OUTBUF + (uint32_t)hMsg->responceLength; i += sizeof(uint32_t) )
    {
        uint32_t regVal;
        regVal = readInbox( hMsg->pMod, i/4 );
        BDBG_LOG(( "MSG< %.3d - %08X",  (i/4), regVal ));
    }
    #endif

    hMsg->pMod->sequenceNumber = (hMsg->pMod->sequenceNumber + 1) & 0xFF;

    BDBG_LEAVE( BHSM_BspMsg_SubmitCommand );
    return BERR_SUCCESS;
}



/* callback called when BSP responds to command. */
static void mailboxInterruptHandler_isr( void* pHsm, int unused )
{
    BHSM_Handle  hHsm;
    uint32_t interruptStatus = 0;
    uint32_t controlStatus = 0;
    uint32_t oLoadRegister = 0;

    BSTD_UNUSED( unused );

    if( pHsm == NULL )
    {
        BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
        return;
    }
    hHsm = (BHSM_Handle)pHsm;

    interruptStatus = BREG_Read32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS );

    if( interruptStatus & MAILBOX_OLOAD_INTERRUPT_STATUS_MASK )
    {
        oLoadRegister = BREG_Read32( hHsm->regHandle, MAILBOX_OLOAD_CONTROL );

        /* Clear OLOADx bit */
        BREG_Write32( hHsm->regHandle, MAILBOX_OLOAD_CONTROL, ( oLoadRegister & (~MAILBOX_OLOAD_CONTROL_MASK) ) );

        /* Clear OLOADx Interrupt. */
        BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS, (~MAILBOX_OLOAD_INTERRUPT_STATUS_MASK) );

       /* if( hHsm->bIsOpen == true )  TODO ... know when BHSM is dead*/
        {
            BHSM_P_BspMsg_Module_t *pModuleData = (BHSM_P_BspMsg_Module_t*)hHsm->pBspMessageModule;
            BKNI_SetEvent( pModuleData->oLoadWait );
        }
    }

    controlStatus = BREG_Read32( hHsm->regHandle, BCHP_BSP_CONTROL_INTR2_CPU_STATUS );

    #if BHSM_DEBUG_BSP_INTERRUPT
    {
        uint32_t oLoadRegisterCheck;
        uint32_t cpuIntrMask;

        oLoadRegisterCheck = BREG_Read32( hHsm->regHandle, MAILBOX_OLOAD_CONTROL );
        cpuIntrMask = BREG_Read32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN );

        BDBG_LOG(("BSP interupt STATUS[%08x] oLoad[%08x][%08x] cpuInt[%08x] control[%08x]", interruptStatus, oLoadRegister, oLoadRegisterCheck, cpuIntrMask, controlStatus ));
    }
    #endif

    if( controlStatus & BCHP_BSP_CONTROL_INTR2_CPU_STATUS_BSP_TO_HOST_INTR_MASK )
    {
        /*if( hHsm->bIsOpen == true ) */ /* TODO .. know when BHSM is dead */
        {
            BDBG_WRN(( "BSP Exception [%X]", controlStatus ));
        }
    }

    return;
}

/* return pointer to last underscore in a string. */
char* cropStr( char* pStr )
{
    char* pUnderScore = pStr;
    unsigned withinBoundry = 120; /*stop run away*/

    if(!pStr) return pStr;

    while( *pStr && --withinBoundry )
    {
        if(*pStr == '_')
        {
            pUnderScore = pStr;
        }
        pStr++;
    }

    if( !withinBoundry ) return NULL;  /*gone out of bounds*/

    return pUnderScore;
}

static void  _ParseBfwVersion( BHSM_BfwVersion *pVersion,
                               uint8_t epochScheme,  /* epoch and scheme. scheme */
                               uint32_t version )
{
    uint8_t scheme = 0; /* 0 for old, 1 for new */
    char strBuf[20] = {0};

    BDBG_ENTER( _ParseBfwVersion );

    BKNI_Memset( pVersion, 0, sizeof(*pVersion) );

    scheme = epochScheme & 0x0f;

    if( scheme == 0 )
    {
        pVersion->version.zeus.major  =  BHSM_ZEUS_VER_MAJOR;
        pVersion->version.zeus.minor  =  BHSM_ZEUS_VER_MINOR;
        /*pre Zeus4.1*/
        pVersion->version.bfw.major   =  ((version >> 12) & 0xF ); /* 4 bits */
        pVersion->version.bfw.minor   =  ((version >>  6) & 0x3F); /* 6 bits */
        /* customerModes       =  ((version >>  0) & 0x3F); */ /* 6 bits */
    }
    else
    {
        pVersion->version.zeus.major    =  ((version >> 28) & 0xF);  /* 4 bits */
        pVersion->version.zeus.minor    =  ((version >> 24) & 0xF);  /* 4 bits */
        pVersion->version.zeus.subminor =  ((version >> 22) & 0x3);  /* 2 bits */
        pVersion->version.bfw.major     =  ((version >> 18) & 0xF);  /* 4 bits */
        pVersion->version.bfw.minor     =  ((version >> 12) & 0x3F); /* 6 bits */
        pVersion->version.bfw.subminor  =  ((version >>  8) & 0xF);  /* 4 bits */
                                                                     /* 2 reserved  bits */
        pVersion->verified              =  ((version >>  4) & 0x3) ? false : true;  /* 2 bits, true of Zero  */

        if( BHSM_ZEUS_VER_MAJOR != pVersion->version.zeus.major || BHSM_ZEUS_VER_MINOR != pVersion->version.zeus.minor )
        {
            /* the zeus version reported by the BFW is different from the zeus version associated with the chip version */
            BDBG_ERR(("Inconsistent security versions. HSM Zeus[%d.%d]", BHSM_ZEUS_VER_MAJOR, BHSM_ZEUS_VER_MINOR ));
        }
    }

    if( scheme == 2 )
    {
        pVersion->firmwareEpoch.valid = true;
        pVersion->firmwareEpoch.value = epochScheme >> 4;

        BKNI_Snprintf( strBuf, sizeof(strBuf), " epoch[%d]", pVersion->firmwareEpoch.value );
    }

    BDBG_LOG(("SECURITY VERSION: Zeus[%d.%d.%d] BFW[%d.%d.%d]%s%s", pVersion->version.zeus.major
                                                                 , pVersion->version.zeus.minor
                                                                 , pVersion->version.zeus.subminor
                                                                 , pVersion->version.bfw.major
                                                                 , pVersion->version.bfw.minor
                                                                 , pVersion->version.bfw.subminor
                                                                 , pVersion->verified?"":" [Alpha]"
                                                                 , strBuf ));
    BDBG_LEAVE( _ParseBfwVersion );
    return;
}
