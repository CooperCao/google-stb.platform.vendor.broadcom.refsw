/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/


#include "bhsm.h"
#include "bchp_bsp_control_intr2.h"
#include "bchp_bsp_glb_control.h"
#include "bsp_s_commands.h"
#include "bhsm_private.h"
#include "bhsm_bsp_msg.h"

/* special support for  HSM ISR  vs. HSM Polling  */
#if BHSM_DEBUG_POLLING
#include "bchp_timer.h"
#endif

#if BHSM_DEBUG_INDIVIDUAL_COMMAND
#include <stdlib.h>
#endif

BDBG_MODULE(BHSMa);

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define BHSM_CTRL_SET_ISR       (0)
#define BHSM_CTRL_SET_POLLING   (1)

#define BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER (0x12435687)

#define BSP_S_DRV_VERSION           (0x10)

/* BSP Message Module data. */
typedef struct BHSM_P_BspMsg_Module_s
{
    bool bspInterfaceBusy;             /* Indicates that HSM is communicating with BSP interface. Used for debug */
    BCMD_cmdType_e currentBspCommand;  /* Current/Last Bsp command executed. */

    unsigned sequenceNumber;          /*incremented by 1 for each command sent to the bsp (from this interface.)*/

    BREG_Handle    hRegister;
    BHSM_BspMsgConfigure_t conf;

    uint32_t *pSecureWord;            /* Pointer to memory location (the size of a uint32_t) that is secure. */
    uint32_t unSecureWord;            /* When there is no secure memory loaction availabe/required, pSecureWord will point to this value. */

}BHSM_P_BspMsg_Module_t;


typedef struct BHSM_P_BspMsg_s
{
    unsigned       magic;                               /* used to validate the handle in function calls */
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
}BHSM_P_BspMsg_t;


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

/* macros to read and write to the mailbox.  */
#define writeOutbox(pModuleData,wordOffset,value) BREG_Write32(((pModuleData)->hRegister), ((pModuleData)->conf.ulInCmdBufAddr +(wordOffset)*4), (value) )
#define readOutbox(pModuleData,wordOffset)        BREG_Read32(((pModuleData)->hRegister),  ((pModuleData)->conf.ulInCmdBufAddr +(wordOffset)*4) )
#define readInbox(pModuleData,wordOffset)         BREG_Read32(((pModuleData)->hRegister),  ((pModuleData)->conf.ulOutCmdBufAddr+(wordOffset)*4) )


void BHSM_BspMsg_DumpOutbox( BHSM_BspMsg_h hMsg )
{
    unsigned wordOffset;
    uint32_t value;

    for( wordOffset = 0; wordOffset < BCMD_BUFFER_BYTE_SIZE/sizeof(uint32_t); wordOffset++ )
    {
        value = readOutbox( hMsg->pMod, wordOffset );
        BDBG_LOG(("> %3d 0x%08X", wordOffset, value ));
    }
}

void BHSM_BspMsg_DumpInbox( BHSM_BspMsg_h hMsg )
{
    unsigned wordOffset;
    uint32_t value;

    for( wordOffset = 0; wordOffset < BCMD_BUFFER_BYTE_SIZE/sizeof(uint32_t); wordOffset++ )
    {
        value = readInbox( hMsg->pMod, wordOffset );
        BDBG_LOG(("< %3d 0x%08X", wordOffset, value ));
    }
}


BERR_Code BHSM_BspMsg_Init( BHSM_Handle hHsm, BHSM_BspMsgConfigure_t *pConfig )
{
    BHSM_P_BspMsg_Module_t *pModuleData;

    BDBG_ENTER( BHSM_BspMsg_Init );

    if( !hHsm || hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER || !pConfig || !hHsm->regHandle )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( pConfig->secureMemory.p && pConfig->secureMemory.size >= sizeof(uint32_t) )
    {
        pModuleData->pSecureWord = pConfig->secureMemory.p;
    }
    else
    {
        pModuleData->pSecureWord = &pModuleData->unSecureWord;
    }

    BDBG_LEAVE( BHSM_BspMsg_Init );

    return BERR_SUCCESS;
}

void BHSM_BspMsg_Uninit( BHSM_Handle hHsm )
{
    if( hHsm->pBspMessageModule )
    {
        BKNI_Free( hHsm->pBspMessageModule );
        hHsm->pBspMessageModule = NULL;
    }
    return;
}


BERR_Code BHSM_BspMsg_Create( BHSM_Handle hHsm,  BHSM_BspMsg_h *phMsg )
{
    BHSM_BspMsg_h hMsg;

    BDBG_ENTER( BHSM_BspMsg_Create );

    if( !hHsm || hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER || !hHsm->pBspMessageModule || !phMsg  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    *phMsg = NULL;

    hMsg = (BHSM_BspMsg_h) BKNI_Malloc( sizeof(BHSM_P_BspMsg_t) );
    if( hMsg == NULL)
    {
        BDBG_ERR(( "Failed to allocate mem for Mail [%d]", sizeof(BHSM_P_BspMsg_t) ));
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    BKNI_Memset( hMsg, 0, sizeof(BHSM_P_BspMsg_t) );

    hMsg->magic = BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER;
    hMsg->isRaw = true;
    hMsg->hHsm = hHsm;
    hMsg->pMod = (BHSM_P_BspMsg_Module_t*)hHsm->pBspMessageModule;
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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER )
    {
        /* invalid parameter. */
        return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    #if BHSM_DEBUG_INDIVIDUAL_COMMAND
    BDBG_SetModuleLevel( "BHSMa", hMsg->level );
    #endif

    hMsg->pMod->bspInterfaceBusy = false;

    hMsg->magic = 0;  /* kill the magic */
    BKNI_Free( hMsg );

    BDBG_LEAVE( BHSM_BspMsg_Destroy );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_Pack8_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t data, char *pCommand)
{
    signed wordOffset;
    signed byteShift;

    BDBG_ENTER( BHSM_BspMsg_Pack8 );

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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



BERR_Code BHSM_BspMsg_PackArray_impl( BHSM_BspMsg_h hMsg, unsigned offset, uint8_t *pData, unsigned length, char *pCommand )
{
    signed wordOffset;
    signed byteShift;
    signed index = 0;

    BDBG_ENTER( BHSM_BspMsg_PackArray );

    if( !hMsg  || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER || !pData  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg  || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER || !pData  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg  || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER || !pData  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER || !pData  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER || !pData  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER || !pData  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER || !pHeader  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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
    for( i = 0; i < (hMsg->hHsm->currentSettings.ulTimeOutInMilSecs * 500); i++ )
    {
        iReady = BREG_Read32( hMsg->hRegister, hMsg->pMod->conf.ulIReadyRegAddr );
        if( iReady & hMsg->pMod->conf.ulIReadyVal )
        {
            break;
        }
        BKNI_Delay( 2 );
    }

    if( i == (hMsg->hHsm->currentSettings.ulTimeOutInMilSecs * 500) )
    {
        hMsg->packErrorCount++;
        BDBG_ERR(( "%s TIMEOUT iREADY[%08X] command[0x%X]", iReady, bspCommand ));
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
        BHSM_BspMsg_Pack8 ( hMsg, BCMD_CommonBufferFields_eVerNum+3  , BSP_S_DRV_VERSION );
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

    if( !hMsg || hMsg->magic != BHSM_P_BSP_MSG_HANDLE_MAGIC_NUMBER  )
    {
         return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

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
        regVal = readOutbox( hMsg->pMod, i );
        BDBG_ERR(("MSG> %.3d - %08X", (i/4), regVal ));
    }
    #endif

    BKNI_ResetEvent( hMsg->pMod->conf.oLoadWait ); /* CTRL-C or any system level signal*/

    /* indicate that message is loaded. */
    BREG_Write32( hRegister,  hMsg->pMod->conf.ulILoadRegAddr, hMsg->pMod->conf.ulILoadVal );

    rc = BKNI_WaitForEvent( hMsg->pMod->conf.oLoadWait, hHsm->currentSettings.ulTimeOutInMilSecs );
    /* If event fails, retry by polling the registers. */
    if( rc != BERR_SUCCESS )
    {
        uint32_t registerValue = 0;
        unsigned timeout;

        #if BHSM_SAGE_BSP_PI_SUPPORT /* if  we're on SAGE */
         #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx                           BCHP_BSP_GLB_CONTROL_GLB_OLOAD1
         #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx_CMD_OLOADx_MASK           BCHP_BSP_GLB_CONTROL_GLB_OLOAD1_CMD_OLOAD1_MASK
        #else
         #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx                           BCHP_BSP_GLB_CONTROL_GLB_OLOAD2
         #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx_CMD_OLOADx_MASK           BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK
        #endif

        for( timeout = 2000; timeout > 0; timeout-- )
        {
            BKNI_Sleep( 5 /*ms*/);

            registerValue = BREG_Read32( hRegister, LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx );

            if( registerValue & LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx_CMD_OLOADx_MASK )
            {
                break;
            }
        }

        if( timeout == 0  )
        {
            BDBG_ERR(("BHSM_BspMsg_SubmitCommand: TIMEOUT on command[0x%02X] GLB_OLOAD[0x%02X]", hMsg->bspCommand, registerValue ));
            return BERR_TRACE( BHSM_STATUS_TIME_OUT );
        }

        BDBG_WRN(("BKNI_WaitForEvent FAILED[%d] [%d] command[0x%02X]. Polled[%d]", rc, hHsm->currentSettings.ulTimeOutInMilSecs, hMsg->bspCommand, timeout ));

        /* Reset the register. */
        BREG_Write32( hRegister, LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx, registerValue & (~LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx_CMD_OLOADx_MASK) );
    }

    BHSM_BspMsg_Get16( hMsg, BCMD_CommonBufferFields_eParamLen, &(hMsg->responceLength) );

    /* read the response header */
    #if BHSM_DEBUG_DUMP_RAW_BSP_COMMAND
    for( i = 0; i < (uint32_t)BCMD_DATA_OUTBUF + (uint32_t)hMsg->responceLength; i += sizeof(uint32_t) )
    {
        uint32_t regVal;
        regVal = readInbox( hMsg->pMod, i/4 );
        BDBG_ERR(( "MSG< %.3d - %08X",  (i/4), regVal ));
    }
    #endif

    hMsg->pMod->sequenceNumber = (hMsg->pMod->sequenceNumber + 1) & 0xFF;

    BDBG_LEAVE( BHSM_BspMsg_SubmitCommand );
    return BERR_SUCCESS;
}
