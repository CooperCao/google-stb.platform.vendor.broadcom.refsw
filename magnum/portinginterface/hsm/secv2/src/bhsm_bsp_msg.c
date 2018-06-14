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

#include <stdlib.h>
#include "bstd.h"
#include "bhsm.h"
#include "bchp.h"
#include "bchp_bsp_cmdbuf.h"
#include "bchp_bsp_control_intr2.h"
#include "bchp_bsp_glb_control.h"
#include "bsp_components.h"
 #include "bsp_headers.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_priv.h"
#include "bchp_bsp_glb_control.h"
#ifdef BHSM_BUILD_HSM_FOR_SAGE
 #include "bchp_bsp_ipi_intr2.h"
#endif
#ifdef BHSM_DEBUG_BSP
  #include "bhsm_bsp_debug.h"
#endif


BDBG_MODULE(BHSMa);

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#ifdef BHSM_BUILD_HSM_FOR_SAGE
    #define MAILBOX_OLOAD_INTERRUPT_ID            BCHP_INT_ID_CREATE( BCHP_BSP_IPI_INTR2_CPU_STATUS, \
                                                                      BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_OLOAD_SHIFT )

    /* SAGE Context */
    #define MAILBOX_IREADY_MASK                   BCHP_BSP_GLB_CONTROL_GLB_IRDY1_CMD_IRDY1_MASK
    #define MAILBOX_IREADY                        BCHP_BSP_GLB_CONTROL_GLB_IRDY1
                                                  /* - wait on it until the BSP is ready for input
                                                     - called form BHSM_BspMsg_Create
                                                     - Read only
                                                     - 0x003b0870  [RO][32] Input Command Buffer Ready
                                                     - bchp_bsp_glb_control.h
                                                  */

    #define MAILBOX_ILOAD_CONTROL_MASK            BCHP_BSP_GLB_CONTROL_GLB_ILOAD1_CMD_ILOAD1_MASK
    #define MAILBOX_ILOAD_CONTROL                 BCHP_BSP_GLB_CONTROL_GLB_ILOAD1
                                                  /*  - write to it to flag that a message is loaded into the BSP buffer.
                                                      - called from BHSM_BspMsg_SubmitCommand
                                                      - Write Only.
                                                      - 0x003b0878  [WO][32] Input Command Buffer 1 Loaded
                                                      - bchp_bsp_glb_control.h
                                                   */

    #define MAILBOX_INT_STATUS_OLOAD_MASK         BCHP_BSP_CONTROL_INTR2_CPU_STATUS_OLOAD1_INTR_MASK
    #define MAILBOX_INT_STATUS                    BCHP_BSP_CONTROL_INTR2_CPU_STATUS
                                                  /*  - wait on it until message BSP is ready
                                                      - called from BHSM_BspMsg_SubmitCommand
                                                      - read only.
                                                      - 0x003b0b00  [RO][32] CPU interrupt Status Register
                                                      - bchp_bsp_control_intr2.h
                                                  */

    #define MAILBOX_INT_CLEAR_OLOAD_MASK          BCHP_BSP_CONTROL_INTR2_CPU_CLEAR_OLOAD1_INTR_MASK
    #define MAILBOX_INT_CLEAR                     BCHP_BSP_CONTROL_INTR2_CPU_CLEAR

    /* mailbox are relocated at BFW_MAILBOX_VA virtual address */
    #include "memory_mailbox.h"
    #define MAILBOX_ADDRESS_IN                    ((volatile uint32_t *)BFW_MAILBOX_VA)
    #define MAILBOX_ADDRESS_OUT                   (MAILBOX_ADDRESS_IN + BHSM_P_MAILBOX_WORD_SIZE)

    #define writeOutbox(regHandle,wordOffset,value)   (MAILBOX_ADDRESS_IN[wordOffset]) = value
    #define readOutbox(regHandle,wordOffset)          (MAILBOX_ADDRESS_IN[wordOffset])
    #define readInbox(regHandle,wordOffset)           (MAILBOX_ADDRESS_OUT[wordOffset])

#else
    #define MAILBOX_OLOAD_INTERRUPT_ID            BCHP_INT_ID_CREATE( BCHP_BSP_CONTROL_INTR2_CPU_STATUS ,\
                                                                      BCHP_BSP_CONTROL_INTR2_CPU_STATUS_OLOAD2_INTR_SHIFT )

    #define MAILBOX_IREADY_MASK                   BCHP_BSP_GLB_CONTROL_GLB_IRDY2_CMD_IRDY2_MASK
    #define MAILBOX_IREADY                        BCHP_BSP_GLB_CONTROL_GLB_IRDY2                   /*OK*/

    #define MAILBOX_ILOAD_CONTROL_MASK            BCHP_BSP_GLB_CONTROL_GLB_ILOAD2_CMD_ILOAD2_MASK
    #define MAILBOX_ILOAD_CONTROL                 BCHP_BSP_GLB_CONTROL_GLB_ILOAD2

    #define MAILBOX_INT_STATUS_OLOAD_MASK         BCHP_BSP_CONTROL_INTR2_CPU_STATUS_OLOAD2_INTR_MASK
    #define MAILBOX_INT_STATUS                    BCHP_BSP_CONTROL_INTR2_CPU_STATUS

    #define MAILBOX_INT_CLEAR_OLOAD_MASK          BCHP_BSP_CONTROL_INTR2_CPU_CLEAR_OLOAD2_INTR_MASK
    #define MAILBOX_INT_CLEAR                     BCHP_BSP_CONTROL_INTR2_CPU_CLEAR

    #define MAILBOX_ADDRESS_IN                   (BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + (BHSM_P_MAILBOX_BYTE_SIZE * 2))
    #define MAILBOX_ADDRESS_OUT                  (BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + (BHSM_P_MAILBOX_BYTE_SIZE * 3))

    /* macros to read and write to the mailbox.  */
    #define writeOutbox(regHandle,wordOffset,value) BREG_Write32( regHandle, (MAILBOX_ADDRESS_IN  + (wordOffset)*4), (value) )
    #define readOutbox(regHandle,wordOffset)        BREG_Read32(  regHandle, (MAILBOX_ADDRESS_IN  + (wordOffset)*4) )
    #define readInbox(regHandle,wordOffset)         BREG_Read32(  regHandle, (MAILBOX_ADDRESS_OUT + (wordOffset)*4) )
#endif


BDBG_OBJECT_ID(BHSM_P_BspMsg);

/* BSP Message Module data. */
typedef struct BHSM_P_BspMsg_Module_s
{
    bool bspInterfaceBusy;     /* Indicates that HSM is communicating with BSP interface. Used for debug */

    unsigned ownerId;          /* incremented by 1 for each command sent to the bsp (from this interface.)*/

    BREG_Handle hRegister;

   #ifdef BHSM_BSP_INTERRUPT_SUPPORT
    BKNI_EventHandle     oLoadWait;
    BINT_CallbackHandle  oLoadCallback;
   #endif

   uint8_t *pSecureMemory;

}BHSM_P_BspMsg_Module_t;


typedef struct BHSM_P_BspMsg
{
    BDBG_OBJECT( BHSM_P_BspMsg )

    BHSM_Handle    hHsm;

    unsigned     component;
    unsigned     command;
    unsigned     continualMode;

    struct{
        bool enable;
        unsigned rsaKeyId;                   /* the RSA keyslot the command is signed against. Valid of "enable" is true. */
        BSTD_DeviceOffset signatureOffset;   /* memory offset to signature. Valid of "enable" is true. */
    }signedCommand;

    uint8_t  *mailBox;  /* shadow mailbox, size of BHSM_P_MAILBOX_BYTE_SIZE */

}BHSM_P_BspMsg;


static void _ParseBfwVersion( BHSM_BfwVersion *pVersion, uint32_t bspFwReleaseVer );
#ifdef BHSM_BSP_INTERRUPT_SUPPORT
static void _MailboxIntHandler_isr( void* pHsm, int unused );
#endif



BERR_Code BHSM_BspMsg_Init( BHSM_Handle hHsm, BHSM_BspMsgInit_t *pParam )
{
    BHSM_P_BspMsg_Module_t *pModuleData;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_BspMsg_Init );

    if( !hHsm ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BSTD_UNUSED( pParam );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( !hHsm->regHandle ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pModuleData = BKNI_Malloc( sizeof(*pModuleData) );
    if( !pModuleData ) { return  BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); }

    hHsm->modules.pBsp = pModuleData;

    BKNI_Memset( pModuleData, 0, sizeof(*pModuleData) );

    pModuleData->hRegister = hHsm->regHandle;
    pModuleData->ownerId = 0;

    pModuleData->pSecureMemory = BHSM_SecureMemory_Allocate( BHSM_P_MAILBOX_BYTE_SIZE );
    if( !pModuleData->pSecureMemory ) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto error; }

   #ifdef BHSM_BSP_INTERRUPT_SUPPORT
    rc = BKNI_CreateEvent( &(pModuleData->oLoadWait) );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto error; }

    rc = BINT_CreateCallback( &(pModuleData->oLoadCallback),
                              hHsm->interruptHandle,
                              MAILBOX_OLOAD_INTERRUPT_ID,
                              _MailboxIntHandler_isr,
                              (void*)hHsm,
                              0x00 );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc );  goto error; }

    rc = BINT_EnableCallback( pModuleData->oLoadCallback );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto error; }
   #endif

    /* initialise mailbox interface */
    BREG_Write32( hHsm->regHandle, MAILBOX_ILOAD_CONTROL, 0 );

   #ifdef BHSM_BUILD_HSM_FOR_HOST
    #ifdef BCHP_BSP_GLB_CONTROL_GLB_RAAGA_INTR_STATUS /* define may not be available/relevant on legacy platforms */
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_RAAGA_INTR_STATUS, 0xFFFFFFFFL );
    #endif
   #endif

    BDBG_LEAVE( BHSM_BspMsg_Init );

    return BERR_SUCCESS;

error:

    BHSM_BspMsg_Uninit( hHsm );

    return rc;
}

void BHSM_BspMsg_Uninit( BHSM_Handle hHsm )
{
    BERR_Code rc;
    BHSM_P_BspMsg_Module_t *pModuleData;

    BDBG_ENTER( BHSM_BspMsg_Uninit );

    if( !hHsm ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( !hHsm->modules.pBsp ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    pModuleData = hHsm->modules.pBsp;

    if( pModuleData->pSecureMemory ) {
        BHSM_SecureMemory_ScrubBeforeFree( pModuleData->pSecureMemory, BHSM_P_MAILBOX_BYTE_SIZE );
        BHSM_SecureMemory_Free( pModuleData->pSecureMemory );
        pModuleData->pSecureMemory = NULL;
    }

   #ifdef BHSM_BSP_INTERRUPT_SUPPORT
    if( pModuleData->oLoadCallback ){
        rc = BINT_DisableCallback( pModuleData->oLoadCallback );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); /* continue */ }

        rc = BINT_DestroyCallback( pModuleData->oLoadCallback );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); /* continue */ }
    }

    if( pModuleData->oLoadWait ) {
        BKNI_DestroyEvent( pModuleData->oLoadWait );
    }
   #endif

    BKNI_Free( pModuleData );
    hHsm->modules.pBsp = NULL;

    BDBG_LEAVE( BHSM_BspMsg_Uninit );
    return;
}


BHSM_BspMsg_h BHSM_BspMsg_Create( BHSM_Handle hHsm, BHSM_BspMsgCreate_t *pParam )
{
    BHSM_BspMsg_h hMsg;
    unsigned timeout;
    uint32_t iReady = 0;

    BDBG_ENTER ( BHSM_BspMsg_Create );

    if( !hHsm ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( !pParam ){ BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( !hHsm->modules.pBsp ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }

    if( hHsm->modules.pBsp->bspInterfaceBusy ) {

        /* Only one BspMsg Message Object should be instantiated at any one time */
        BERR_TRACE( BERR_NOT_AVAILABLE );
        return NULL;
    }
    hHsm->modules.pBsp->bspInterfaceBusy = true;

    hMsg = (BHSM_BspMsg_h) BKNI_Malloc( sizeof(BHSM_P_BspMsg) );
    if( hMsg == NULL) { BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); return NULL; }

    BKNI_Memset( hMsg, 0, sizeof(BHSM_P_BspMsg) );
    BDBG_OBJECT_SET( hMsg, BHSM_P_BspMsg );

    hMsg->hHsm = hHsm;
    hMsg->mailBox = hHsm->modules.pBsp->pSecureMemory;

    /* Wait until BSP can accept input */
    for( timeout = 1000; timeout; timeout-- )
    {
        iReady = BREG_Read32( hHsm->regHandle, MAILBOX_IREADY );

        if( iReady & MAILBOX_IREADY_MASK ) { break; }

        BKNI_Delay( 2 );
    }

    if( timeout == 0 )
    {
        BDBG_ERR(( "BSP TIMEOUT iREADY[%08X]", iReady ));
        BERR_TRACE( BHSM_STATUS_IRDY_ERR );             /* continue, at risk. */
    }

    pParam->pSend    = &hMsg->mailBox[sizeof(Bsp_CmdHeader_InFields_t)];
    pParam->pReceive = &hMsg->mailBox[sizeof(Bsp_CmdHeader_OutFields_t)];

    BDBG_LEAVE( BHSM_BspMsg_Create );
    return hMsg;
}


BERR_Code BHSM_BspMsg_Destroy ( BHSM_BspMsg_h hMsg )
{
    BHSM_Handle hHsm;

    BDBG_ENTER( BHSM_BspMsg_Destroy );

    if( !hMsg ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    hHsm = hMsg->hHsm;

    hHsm->modules.pBsp->bspInterfaceBusy = false;

    BDBG_OBJECT_DESTROY( hMsg, BHSM_P_BspMsg );
    BKNI_Free( hMsg );

    BDBG_LEAVE( BHSM_BspMsg_Destroy );
    return BERR_SUCCESS;
}

BERR_Code BHSM_BspMsg_Configure( BHSM_BspMsg_h hMsg, BHSM_BspMsgConfigure_t *pParam )
{
    BDBG_ENTER( BHSM_BspMsg_Configure );

    if( !hMsg ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pParam ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    hMsg->component = pParam->component;
    hMsg->command = pParam->command;
    hMsg->continualMode = pParam->continualMode;

    hMsg->signedCommand.enable = pParam->signedCommand.enable;
    hMsg->signedCommand.rsaKeyId = pParam->signedCommand.rsaKeyId;
    hMsg->signedCommand.signatureOffset = pParam->signedCommand.signatureOffset;

    BDBG_LEAVE( BHSM_BspMsg_Configure );
    return BERR_SUCCESS;
}


BERR_Code BHSM_BspMsg_SubmitCommand( BHSM_BspMsg_h hMsg, uint16_t *pBspStatus )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_Handle hHsm;
    uint32_t regValue = 0;
    unsigned i = 0;
    unsigned commandLength = 0;
    Bsp_CmdHeader_InFields_t *pSendHeader = NULL;
    Bsp_CmdHeader_OutFields_t *pReceiveHeader = NULL;

    if( !hMsg ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pBspStatus ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( hMsg, BHSM_P_BspMsg );

    hHsm = hMsg->hHsm;

    BREG_Write32( hHsm->regHandle, MAILBOX_INT_CLEAR, MAILBOX_INT_CLEAR_OLOAD_MASK );

    /* set the mailbox header.*/
    pSendHeader = (Bsp_CmdHeader_InFields_t*)&hMsg->mailBox[0];

    BKNI_Memset( pSendHeader, 0, sizeof(*pSendHeader) );
    pSendHeader->ownerId            = hHsm->modules.pBsp->ownerId;
    if( hMsg->signedCommand.enable )
    {
        pSendHeader->signedFlag      = true;
        pSendHeader->secondTierKeyId = (uint8_t)hMsg->signedCommand.rsaKeyId;
        pSendHeader->signCmdAddrMsb  = (uint8_t)(hMsg->signedCommand.signatureOffset >> 32);
        pSendHeader->signCmdAddr     = (uint32_t)(hMsg->signedCommand.signatureOffset & 0xFFFFFFFF);
    }
    pSendHeader->cmdId              = hMsg->command;
    pSendHeader->cmdComponent       = hMsg->component;
    pSendHeader->cmdGlitch          = (((~hMsg->component)&0xFF) << 8) | ((~hMsg->command)&0xFF);

    /* Copy data to mailbox registers. */
    for( i = 0; i < BHSM_P_MAILBOX_WORD_SIZE; i++ )
    {
        BKNI_Memcpy( &regValue, &hMsg->mailBox[i*4], 4 );
        writeOutbox( hHsm->regHandle, i, regValue );
        if( regValue ) commandLength = i;
    }

   #ifdef BHSM_DEBUG_BSP
    BHSM_BspDebug_PrintCommand( hHsm, (uint32_t*)hMsg->mailBox );
   #endif

    for( i = 0; i <= commandLength; i++ )
    {
        BKNI_Memcpy( &regValue, &hMsg->mailBox[i*4], 4 );
        BDBG_MSG(("> %02d 0x%08X", i, regValue ));
    }
    BDBG_MSG(("Rest of mailbox is zeros."));

    /* indicate that message is loaded. */
    BREG_Write32( hHsm->regHandle, MAILBOX_ILOAD_CONTROL, MAILBOX_ILOAD_CONTROL_MASK );

  #ifdef BHSM_BSP_INTERRUPT_SUPPORT
    rc = BKNI_WaitForEvent( hHsm->modules.pBsp->oLoadWait, 5000 );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
  #else
    {
        unsigned timeout;

        for( timeout = 2000; timeout; timeout-- )
        {
            BKNI_Sleep( 5/*ms*/);

            regValue = BREG_Read32( hHsm->regHandle, MAILBOX_INT_STATUS );
            if( regValue & MAILBOX_INT_STATUS_OLOAD_MASK ) { break; }
        }

        if( timeout == 0 )
        {
            BDBG_ERR(("BSP TIMEOUT OLOAD_CONTROL[0x%02X]", regValue ));
            return BERR_TRACE( BHSM_STATUS_TIME_OUT );
        }
    }
  #endif

    /* read the mailbox back into buffer. */
    for( i = 0; i < BHSM_P_MAILBOX_WORD_SIZE; i++ )
    {
        regValue = readInbox( hHsm->regHandle, i );
        BKNI_Memcpy( &hMsg->mailBox[i*4], &regValue, 4 );
        if( regValue ) { commandLength = i; }
    }

   #ifdef BHSM_DEBUG_BSP
    BHSM_BspDebug_PrintResponse( hHsm, (uint32_t*)hMsg->mailBox );
   #endif

    pReceiveHeader = (Bsp_CmdHeader_OutFields_t*)&hMsg->mailBox[0];

    if( !hHsm->bfwVersionValid )
    {
        _ParseBfwVersion( &hHsm->bfwVersion, pReceiveHeader->bspFwReleaseVer );
        hHsm->bfwVersionValid = true;
    }

    for( i = 0; i < (unsigned)(5 + (pReceiveHeader->paramLen/4)); i++ )
    {
        uint32_t regVal;
        BKNI_Memcpy( &regVal, &hMsg->mailBox[i*4], 4 );
        BDBG_MSG((" <%02d 0x%08X", i, regVal ));
    }

    if( pReceiveHeader->ownerId != hHsm->modules.pBsp->ownerId )
    {
         BERR_TRACE( BHSM_STATUS_BSP_ERROR );
    }

    hHsm->modules.pBsp->ownerId = (hHsm->modules.pBsp->ownerId + 1) & 0xFF;

    *pBspStatus = pReceiveHeader->cmdStatus;

    BDBG_LEAVE( BHSM_BspMsg_SubmitCommand );
    return BERR_SUCCESS;
}

static void  _ParseBfwVersion( BHSM_BfwVersion *pVersion, uint32_t bspFwReleaseVer )
{
    unsigned verifiedBits;

    if( !pVersion ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    BKNI_Memset( pVersion, 0, sizeof(*pVersion) );

    pVersion->version.zeus.major    = (bspFwReleaseVer >> 28) & 0x0f;  /*Bit[31:28], 4 bits */
    pVersion->version.zeus.minor    = (bspFwReleaseVer >> 24) & 0x0f;  /*Bit[27:24], 4 bits */
    pVersion->version.zeus.subminor = (bspFwReleaseVer >> 22) & 0x03;  /*Bit[23:22], 2 bits */

    pVersion->version.bfw.major     = (bspFwReleaseVer >> 18) & 0x0f;  /*Bit[21:18], 4 bits */
    pVersion->version.bfw.minor     = (bspFwReleaseVer >> 12) & 0x3f;  /*Bit[17:12], 6 bits */
    pVersion->version.bfw.subminor  = (bspFwReleaseVer >> 8) & 0x0f;   /*Bit[11:8],  4 bits */
                                                                       /*Bit[7:6],  2 bits reserved */
    verifiedBits                    = (bspFwReleaseVer >> 4) & 0x03;   /*Bit[5:4],  2 bits */
    pVersion->verified = (verifiedBits == 0)?true:false;

    BDBG_LOG(("SECURITY VERSION: Zeus[%d.%d.%d] BFW[%d.%d.%d] %s", pVersion->version.zeus.major
                                                                 , pVersion->version.zeus.minor
                                                                 , pVersion->version.zeus.subminor
                                                                 , pVersion->version.bfw.major
                                                                 , pVersion->version.bfw.minor
                                                                 , pVersion->version.bfw.subminor
                                                                 , pVersion->verified?"":"[Alpha]" ));

    if( BHSM_ZEUS_VER_MAJOR != pVersion->version.zeus.major ||
        BHSM_ZEUS_VER_MINOR != pVersion->version.zeus.minor )
    {
         /* the zeus version reported by the BFW is different from the zeus version associated with the chip version */
         BDBG_ERR(("Inconsistent security versions. HSM Zeus[%d.%d]", BHSM_ZEUS_VER_MAJOR, BHSM_ZEUS_VER_MINOR ));
    }

    return;
}


#ifdef BHSM_BSP_INTERRUPT_SUPPORT
/* callback called when BSP responds to command. */
static void _MailboxIntHandler_isr( void* pHsm, int unused )
{
    BHSM_Handle  hHsm = (BHSM_Handle)pHsm;
    BSTD_UNUSED( unused );

    if( !hHsm ) { BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); return; }
    if( !hHsm->modules.pBsp ){ BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); return; }

    BREG_Write32_isr( hHsm->regHandle, MAILBOX_INT_CLEAR, MAILBOX_INT_CLEAR_OLOAD_MASK );

    BKNI_SetEvent_isr( hHsm->modules.pBsp->oLoadWait );

    return;
}
#endif
