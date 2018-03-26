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

#include "bhsm.h"
#include "bhsm_datatypes.h"
#include "bhsm_private.h"
#include "bhsm_keyslots.h"
#include "bhsm_keyladder_enc.h"
#include "bhsm_keyladder_enc_private.h"
#include "bhsm_bsp_msg.h"

#define BHSM_KEYLADDER_HDCP_ROOT_KEY_SOURCE_MASK (0x80)


BDBG_MODULE(BHSM);

BERR_Code  BHSM_InitialiseKeyLadders ( BHSM_Handle hHsm )
{
    BERR_Code rc = BERR_SUCCESS;
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    unsigned i = 0;
    #endif

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( ( rc = BHSM_LoadVklOwnershipFromBsp( hHsm ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    /* invalidate VKLs that have HOST ownership. */
    for( i = 0; i < BHSM_MAX_VKL; i++ )
    {
        if( !hHsm->vkl[i].neverUsed && hHsm->vkl[i].client == BHSM_ClientType_eHost )
        {
            /* invalidate it. */
            BHSM_InvalidateVkl_t invalidateVkl;

            BDBG_MSG(("VKL [%d] Invalidating HOST VKL.", i ));

            BKNI_Memset( &invalidateVkl, 0, sizeof(invalidateVkl) );
            invalidateVkl.virtualKeyLadderID = (i | BHSM_VKL_ID_ALLOCATION_FLAG);
            invalidateVkl.bInvalidateVkl = true;
            invalidateVkl.keyLayer = BCMD_KeyRamBuf_eKey3;
            rc = BHSM_InvalidateVKL( hHsm, &invalidateVkl );
            if( rc != BERR_SUCCESS )
            {
                BERR_TRACE( rc ); /* failed to invalidate VKL. Continue with error. */
                continue;
            }

            hHsm->vkl[i].neverUsed = true;
            hHsm->vkl[i].client = BHSM_ClientType_eNone;
        }
    }
    #endif

BHSM_P_DONE_LABEL:

    return rc;
}


BERR_Code BHSM_AllocateVKL(
    BHSM_Handle             hHsm,
    BHSM_AllocateVKLIO_t    *pAllocVkl
)
{
    BERR_Code rc = BERR_SUCCESS;
    unsigned  index = 0;
    bool      foundNeverUsedVkl = false;
    unsigned  neverUsedVkl = 0;
    bool      foundReusableVkl = false;
    unsigned  reusableVkl = 0;

    BDBG_ENTER(BHSM_AllocateVKL);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pAllocVkl == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    if( ( pAllocVkl->client != BHSM_ClientType_eHost ) &&
        ( pAllocVkl->client != BHSM_ClientType_eSAGE ) )
    {
        /* invalid client type specified.  */
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
   #endif

    /* Find a free of reusable VKL */
    for( index = 0; index < BHSM_MAX_VKL; index++ )
    {
        if( hHsm->vkl[index].free ) /* available */
        {
          #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
            if( hHsm->vkl[index].neverUsed )
            {
                foundNeverUsedVkl = true;
                neverUsedVkl = index;
            } else if( hHsm->vkl[index].client == pAllocVkl->client )     /*   owned by requesting client */
            {
                foundReusableVkl = true;
                reusableVkl = index;
                break;
            }
          #else /* pre Zeus 3 */
            if( hHsm->vkl[index].neverUsed )
            {
                foundNeverUsedVkl = true;
                neverUsedVkl = index;
            } else if( pAllocVkl->bNewVKLCustSubModeAssoc == false )
            {
                /* check if the customer sub mode is the same. */
                if( hHsm->vkl[index].custSubMode == pAllocVkl->customerSubMode )
                {
                    foundReusableVkl = true;
                    reusableVkl = index;
                    break;
                }
            }
          #endif
        }
    }

    if( foundReusableVkl )
    {
        pAllocVkl->allocVKL  = reusableVkl;
        hHsm->vkl[reusableVkl].free = false;
        hHsm->vkl[reusableVkl].neverUsed = false;
        hHsm->vkl[reusableVkl].custSubMode = pAllocVkl->customerSubMode;
        hHsm->vkl[reusableVkl].client = pAllocVkl->client;

    }
    else if( foundNeverUsedVkl )
    {
        pAllocVkl->allocVKL = neverUsedVkl;
        hHsm->vkl[neverUsedVkl].free = false;
        hHsm->vkl[neverUsedVkl].neverUsed = false;
        hHsm->vkl[neverUsedVkl].custSubMode = pAllocVkl->customerSubMode;
        hHsm->vkl[neverUsedVkl].client = pAllocVkl->client;
    }
    else
    {
        rc = BERR_TRACE( BHSM_STATUS_VKL_ALLOC_ERR );  /* failed to allocate key ladder */
    }

#if BHSM_ENFORCE_VKL_ALLOCATION
    if ( rc == BERR_SUCCESS)
    {
        pAllocVkl->allocVKL  |= BHSM_VKL_ID_ALLOCATION_FLAG;
    }
#endif

    pAllocVkl->unStatus = rc;

    BDBG_LEAVE(BHSM_AllocateVKL);

    return rc;
}

BCMD_VKLID_e BHSM_RemapVklId( BCMD_VKLID_e vkl )
{
#if BHSM_ENFORCE_VKL_ALLOCATION
    if(!(vkl & BHSM_VKL_ID_ALLOCATION_FLAG ))
    {
        if (vkl != BCMD_VKL_KeyRam_eMax)
        {
            BDBG_WRN(("!!! Issue: VKL#%d has not been allocated by Nexus or HSM VKL API. It is likely to cause confict with others using the same VKL.", vkl));
        }
        return vkl;
    }
    else
    {
        /* return vkl & (0x7F); */
        return vkl & (~BHSM_VKL_ID_ALLOCATION_FLAG);
    }

#else
    return vkl;
#endif
}

void BHSM_FreeVKL(  BHSM_Handle   hHsm,
                    BCMD_VKLID_e  vkl  )
{

    BDBG_ENTER( BHSM_FreeVKL );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    vkl = BHSM_RemapVklId(vkl);

    if( vkl >= BHSM_MAX_VKL )
    {
        /* invalid virtual keyladder ID specified. */
        (void)BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
        return;
    }

    hHsm->vkl[vkl].free = true;

    BDBG_MSG(("VKL [%d]  is freed.", vkl ));

    BDBG_LEAVE( BHSM_FreeVKL );

    return;
}

BERR_Code BHSM_InvalidateVKL( BHSM_Handle hHsm,
                              BHSM_InvalidateVkl_t * pConfig )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    unsigned vklId = 0;
    uint8_t status;

    BDBG_ENTER( BHSM_InvalidateVKL );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pConfig == NULL )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    vklId = BHSM_RemapVklId(pConfig->virtualKeyLadderID);
    if( vklId >= BCMD_VKL_KeyRam_eMax )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );

    #define HSM_INVALIDATE_VKL (BCMD_cmdType_eSESSION_INVALIDATE_KEY)
    BHSM_BspMsg_Header( hMsg, HSM_INVALIDATE_VKL, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eVKLID, vklId );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeyFlag, BCMD_InvalidateKey_Flag_eSrcKeyOnly );

  #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    if( hHsm->firmwareVersion.bseck.major >= 4 )
    {
        if( pConfig->bInvalidateVkl )
        {
             /* clear all key layers and ownership */
            BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eFreeVKLOwnerShip, 1 );
            BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eAllKeyLayer, 1 );
        }
        else if( pConfig->bInvalidateAllKeyLayers )
        {
            /* clear all key layers */
            BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eAllKeyLayer, 1 );
        }
    }
  #endif

    /* clear single key layer. Ignored if bInvalidateAllKeyLayers and bInvalidateVkl is set. */
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeyLayer, pConfig->keyLayer );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 )
    {
        BDBG_ERR(("%s BSP error status[0x%02X]. VKL[%d] )", BSTD_FUNCTION, status, vklId ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    if( hHsm->firmwareVersion.bseck.major >= 4 && pConfig->bInvalidateVkl )
    {
        hHsm->vkl[vklId].client = BHSM_ClientType_eNone;
        hHsm->vkl[vklId].neverUsed = true;
    }
   #endif

BHSM_P_DONE_LABEL:

    if( hMsg )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE(BHSM_InvalidateVKL);
    return rc;
}

BERR_Code BHSM_GenerateRouteKey(
    BHSM_Handle                hHsm,
    BHSM_GenerateRouteKeyIO_t *pGrk
)
{
    BERR_Code                     errCode = BERR_SUCCESS;
#if !HSM_IS_ASKM_28NM_ZEUS_4_0
    BHSM_P_M2MKeySlotAlgorithm_t *pM2MKeySlotAlgorithm = NULL;
#endif
    BHSM_P_XPTKeySlotAlgorithm_t *pXPTKeySlotAlgorithm = NULL;
    BHSM_P_AskmData_t            *pAskmData = NULL;
    unsigned                      unKeySlotNum  = 0;
    BHSM_P_CAKeySlotInfo_t       *pKeyslot = NULL;
#if HSM_IS_ASKM_28NM_ZEUS_4_1
    uint32_t                      cntrlWordHi;
    uint32_t                      cntrlWordLo;
    unsigned                      extKsNum = 0; /*external keyslot number*/
#endif
    BHSM_BspMsg_h                 hMsg = NULL;
    BHSM_BspMsgHeader_t           header;
    uint8_t                       byte = 0;
    uint8_t                       status = 0;

    BDBG_ENTER(BHSM_GenerateRouteKey);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pGrk == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    /* Checking if the Vkl Id is allocated or hardcoded */
    if( pGrk->keyLadderSelect != BCMD_KeyLadderSelection_eReserved2 &&
        BHSM_RemapVklId ( pGrk->virtualKeyLadderID ) >= BHSM_MAX_VKL )
    {
        /* invalid virtual keyladder ID specified. */
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR );
    }

    if (pGrk->subCmdID == (unsigned int)BCMD_VKLAssociationQueryFlag_eQuery)
    {
        /* use BHSM_LoadVklOwnershipFromBsp to query VKL configuration */
        return  BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR );
    }

    unKeySlotNum  = pGrk->unKeySlotNum;

    errCode = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( errCode != BERR_SUCCESS )
    {
        BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY, &header );


#if HSM_IS_ASKM_40NM_ZEUS_2_0
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderSelection, pGrk->keyLadderSelect );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eASKM3DesKLRootKeySwapEnable, pGrk->bASKM3DesKLRootKeySwapEnable );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLAssociationQuery, pGrk->subCmdID );
#endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderType, pGrk->keyLadderType );

    if( !pGrk->askm.configurationEnable && !pGrk->sageAskmConfigurationEnable )
    {
        /* Get pointer to structure BHSM_P_KeySlotAlgorithm_t and BHSM_P_KeySlotIDData */
        switch( pGrk->keyDestBlckType )
        {
            case BCMD_KeyDestBlockType_eMem2Mem :

                /* Parameter checking on unKeySlotNum */
               #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
                if( unKeySlotNum >= BCMD_MAX_M2M_KEY_SLOT )
                {
                    errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
                    goto BHSM_P_DONE_LABEL;
                }

                pM2MKeySlotAlgorithm = &(hHsm->aunM2MKeySlotInfo[unKeySlotNum].aKeySlotAlgorithm[pGrk->keyDestEntryType]);
                pAskmData       = &(hHsm->aunM2MKeySlotInfo[unKeySlotNum].askmData[pGrk->keyDestEntryType]);

                if( (pGrk->bIsRouteKeyRequired == true) && (pM2MKeySlotAlgorithm->configured == false) )
                {
                    errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );
                    goto BHSM_P_DONE_LABEL;
                }
                break;
              #endif

            case BCMD_KeyDestBlockType_eCA :
            case BCMD_KeyDestBlockType_eCPDescrambler :
            case BCMD_KeyDestBlockType_eCPScrambler :

                if( ( errCode =  BHSM_P_GetKeySlot( hHsm,
                                                   &pKeyslot,
                                                    pGrk->caKeySlotType,
                                                    pGrk->unKeySlotNum ) ) != BERR_SUCCESS )
                {
                    errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );
                    goto BHSM_P_DONE_LABEL;
                }

                pXPTKeySlotAlgorithm = &(pKeyslot->aKeySlotAlgorithm[pGrk->keyDestEntryType]);
                pAskmData            = &(pKeyslot->askmData[pGrk->keyDestEntryType]);

                BDBG_ASSERT( pKeyslot );
                BDBG_ASSERT( pXPTKeySlotAlgorithm );
                BDBG_ASSERT( pAskmData );

                if( pGrk->bIsRouteKeyRequired == true )
                {
                    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR, !(pXPTKeySlotAlgorithm->configured));
                }
               #if HSM_IS_ASKM_28NM_ZEUS_4_0
                /* let's modify the keyDestBlockType setting for M2M DMA  -- Zeus 4.0 */
                if( pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem )
                {
                    if (pKeyslot->bDescrambling)
                    {
                        pGrk->keyDestBlckType = BCMD_KeyDestBlockType_eCPDescrambler;
                    }
                    else
                    {
                        pGrk->keyDestBlckType = BCMD_KeyDestBlockType_eCPScrambler;
                    }
                }
               #endif
                break;

            default :
                /* no checking for other types; no key slot associated */
                break;
        }

        BDBG_MSG(("GRK keySlot[%d] type[%d] dest[%d] ASKM[%d]", unKeySlotNum \
                                                            , pGrk->keyDestEntryType \
                                                            , pGrk->keyDestBlckType \
                                                            , pGrk->bASKMModeEnabled ));
    }

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eRootKeySrc, pGrk->rootKeySrc );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCustomerSel, pGrk->customerSubMode );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eASKMSel, pGrk->bASKMModeEnabled?1:0 );
#if  HSM_IS_ASKM_40NM_ZEUS_2_0
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eNoKeyGenFlag, pGrk->keyGenMode );
#endif
    /* limit Swizzle1 Index to 5 bits only */
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSwizzle1IndexSel, (pGrk->ucSwizzle1Index & 0x1F) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSwizzleType, pGrk->swizzleType );


    byte =  (pGrk->ucCustKeyLow & 0x3F);
    byte |= (pGrk->bUseCustKeyLowDecrypt?1:0) << 6;
    byte |= (pGrk->cusKeySwizzle0aEnable?1:0) << 7;
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCusKeySelL, byte );

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    if (pGrk->cusKeySwizzle0aEnable)
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eOwnerIDSelect, pGrk->cusKeySwizzle0aVariant ); /*8*/
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKey2GenVersion, pGrk->cusKeySwizzle0aVersion ); /*24*/
    }
    else if(pGrk->rootKeySrc == BCMD_RootKeySrc_eASKMGlobalKey)
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eOwnerIDSelect, pGrk->globalKeyOwnerId ); /*8*/
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eASKMGlobalKeyIndex, pGrk->AskmGlobalKeyIndex ); /*16*/
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKey2GenVersion, pGrk->globalKeyVersion ); /*24*/
    }
#elif BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSwizzle0aType, pGrk->cusKeySwizzle0aVariant ); /*8*/
    BHSM_BspMsg_Pack8( hMsg, ((9<<2)+1), pGrk->cusKeySwizzle0aVersion );  /*16*/
#endif


    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyVarL, pGrk->ucKeyVarLow );

    byte =  (pGrk->ucCustKeyHigh & 0x3F);
    byte |= (pGrk->bUseCustKeyHighDecrypt?1:0) << 6;
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCusKeySelH, byte );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyVarH, pGrk->ucKeyVarHigh );
    if( pGrk->keyLadderSelect !=  BCMD_KeyLadderSelection_eReserved2 )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLID, BHSM_RemapVklId(pGrk->virtualKeyLadderID ));
    }

    #if ( BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2) )
    if( pGrk->keyTweak == BHSM_KeyTweak_eDupleConnect )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSourceDuple, BHSM_RemapVklId( pGrk->sourceDupleKeyLadderId ));
    }
    #endif

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLayer, pGrk->keyLayer );
   #if ( ( BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0) ) || BHSM_SECURE_KEY_TWEAK_MK )
    #if ( ( BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0) ) || BHSM_SECURE_KEY_TWEAK_MK )
    if ( pGrk->customerSubMode == BCMD_CustomerSubMode_eSCTE52_CA_5 ) {
        pGrk->keyTweak = BHSM_KeyTweak_eDSK;
    }
    #endif

   #if ( BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2) )
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCwProtectionKeyIvSource, pGrk->protectionKeyIvSource );
   #endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyTweak, pGrk->keyTweak );

   #elif ( BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0) )
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyTweak, pGrk->key3Op ); /* BCMD_GenKey_InCmd_eKey3Operation deprecated from Zeus2,1 */
   #else
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKey3Operation, pGrk->key3Op );
   #endif

    #if ( BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2) )
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eApplyKKCV, pGrk->applyKeyContribution?1:0 );
    #endif

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderOpera, pGrk->bIsKeyLadder3DESEncrypt );

    BHSM_BspMsg_PackArray( hMsg,
                           BCMD_GenKey_InCmd_eProcIn + BHSM_GEN_ROUTE_KEY_DATA_LEN - pGrk->ucKeyDataLen,
                           pGrk->aucKeyData,
                           pGrk->ucKeyDataLen );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeySize, pGrk->keySize );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSwapAESKey, pGrk->bSwapAesKey?1:0 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eRouteKeyFlag, pGrk->bIsRouteKeyRequired );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eBlkType, pGrk->keyDestBlckType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eEntryType, pGrk->keyDestEntryType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eIVType, pGrk->keyDestIVType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeySlotType, pGrk->caKeySlotType );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeySlotNumber, pGrk->unKeySlotNum );
#if HSM_IS_ASKM_40NM_ZEUS_3_0
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSC01ModeWordMapping, (pGrk->SC01ModeMapping & 0xFF) );
#endif
#if HSM_IS_ASKM_40NM_ZEUS_2_0
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eGPipeSC01Value, pGrk->GpipeSC01Val );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eRPipeSC01Value, pGrk->RpipeSC01Val );
#endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyMode, pGrk->keyMode );

    if ( pXPTKeySlotAlgorithm &&
        ( (pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eCA)            ||
          (pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eCPDescrambler) ||
          (pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler)   )  )
    {
       #if HSM_IS_ASKM_28NM_ZEUS_4_1
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord0, pKeyslot->ulGlobalControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord1, pKeyslot->ulGlobalControlWordLo );

        if (pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eCA)
        {
            cntrlWordHi = pXPTKeySlotAlgorithm->ulCAControlWordHi;
            cntrlWordLo = pXPTKeySlotAlgorithm->ulCAControlWordLo;
        }
        else if (pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler)
        {
            cntrlWordHi = pXPTKeySlotAlgorithm->ulCPSControlWordHi;
            cntrlWordLo = pXPTKeySlotAlgorithm->ulCPSControlWordLo;
        }
        else
        {
            cntrlWordHi = pXPTKeySlotAlgorithm->ulCPDControlWordHi;
            cntrlWordLo = pXPTKeySlotAlgorithm->ulCPDControlWordLo;
        }

        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord2, cntrlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord3, cntrlWordLo );
       #else
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord0, pXPTKeySlotAlgorithm->ulCAControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord1, pXPTKeySlotAlgorithm->ulCAControlWordLo );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord2, pXPTKeySlotAlgorithm->ulCPSControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord3, pXPTKeySlotAlgorithm->ulCPSControlWordLo );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord4, pXPTKeySlotAlgorithm->ulCPDControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord5, pXPTKeySlotAlgorithm->ulCPDControlWordLo );
       #endif
    }

#if !HSM_IS_ASKM_28NM_ZEUS_4_0
    else if ( pM2MKeySlotAlgorithm && pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem )
    {
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord0, pM2MKeySlotAlgorithm->ulM2MControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eCtrlWord1, pM2MKeySlotAlgorithm->ulM2MControlWordLo );
    }
#endif

    /* more stuffs needed for ASKM mode or space filler */
    /* To support SAGE dedicated Usage key ladder (Zeus 3.0 and up), ASKM parameters are loaded  */
    /* directly into the command buffer from SAGE app  */
#if HSM_IS_ASKM_40NM_ZEUS_3_0
    if ((pGrk->bASKMModeEnabled) || (pGrk->cusKeySwizzle0aEnable))
#else
    if (pGrk->bASKMModeEnabled)
#endif
    {
        if( pGrk->sageAskmConfigurationEnable || pGrk->askm.configurationEnable )
        {
            if( pGrk->sageAskmConfigurationEnable ) /*DEPRECATED, to be removed.*/
            {
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSTBOwnerIDSel, pGrk->sageSTBOwnerID );
                BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eCAVendorID, pGrk->sageCAVendorID );
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eModuleID, pGrk->sageModuleID );
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eTestKeySel, pGrk->sageMaskKeySelect );
            }
            else if( pGrk->askm.configurationEnable  )
            {
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSTBOwnerIDSel, pGrk->askm.stbOwnerId );
                BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eCAVendorID, pGrk->askm.caVendorId );
               #if BHSM_BUILD_HSM_FOR_SAGE /* if  we're on SAGE */
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCAVendorIDExtension, pGrk->askm.caVendorIdExtension );
               #endif
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eModuleID, pGrk->askm.moduleId );
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eTestKeySel, pGrk->askm.maskKeySelect );
            }
        }
        else
        {
            if( pAskmData  )
            {
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSTBOwnerIDSel, pAskmData->ulSTBOwnerIDSelect );
                BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eCAVendorID, pAskmData->ulCAVendorID );
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eTestKeySel, pAskmData->maskKeySelect );
                BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eModuleID, pAskmData->ulModuleID );
            }
        }
    }

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eHwKlLength, pGrk->hwklLength );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eHwKlDestinationAlg, pGrk->hwklDestAlg );
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eReserved_40_1, pGrk->bHWKLVistaKeyGenEnable?1:0 );
    #endif

    BHSM_BspMsg_PackArray( hMsg,
                           BCMD_GenKey_InCmd_eReserved_41_0,
                           pGrk->activationCode,
                           BHSM_KL_ACTCODE_LEN );

    #if HSM_IS_ASKM_28NM_ZEUS_4_0
    if ( pXPTKeySlotAlgorithm &&
         ((pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eCA)            ||
          (pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eCPDescrambler) ||
          (pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler)   ||
          (pGrk->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem) ) )
    {
        if( pXPTKeySlotAlgorithm->externalKeySlot.valid )    /* Set external Key Slot parameters.  */
        {
            extKsNum = pXPTKeySlotAlgorithm->externalKeySlot.slotNum;
            if( extKsNum >= BHSM_EXTERNAL_KEYSLOTS_MAX )
            {
                errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); /* invalid external keyslot number */
                goto BHSM_P_DONE_LABEL;
            }

            if( hHsm->externalKeySlotTable[extKsNum].iv.valid )
            {
                BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eExtIVPtr, ( hHsm->externalKeySlotTable[extKsNum].slotPtr +
                                                                         hHsm->externalKeySlotTable[extKsNum].iv.offset ) );
            }
        }
    }
    #endif

    #if HSM_IS_ASKM_40NM_ZEUS_3_0
    BHSM_BspMsg_Pack8( hMsg,  BCMD_GenKey_InCmd_eReserved_46_3, (pGrk->bResetPKSM?1:0) );
    BHSM_BspMsg_Pack8( hMsg,  BCMD_GenKey_InCmd_eReserved_47_3, pGrk->PKSMInitSize );
    BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eReserved_48_2, pGrk->PKSMcycle );
    #endif

    #if ( ( BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(2,0) ) || ( BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2) )  ||  BHSM_SECURE_KEY_TWEAK_MK )
    if (pGrk->keyTweak == BHSM_KeyTweak_eDSK) {
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eReservedDataForDSK+(4*0), 0x430351E9 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eReservedDataForDSK+(4*1), 0x6E5C8A83 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eReservedDataForDSK+(4*2), 0xD986E867 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_GenKey_InCmd_eReservedDataForDSK+(4*3), 0x5E41E3A8 );
    }
    #endif

#endif /* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0) */

    errCode = BHSM_BspMsg_SubmitCommand( hMsg );
    if( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE(errCode);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pGrk->unStatus = (uint32_t)status;

    if( pGrk->unStatus != 0 )
    {
        errCode = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

    /* The command was successful, so update the VKL-CustSubMode association */
    if( pGrk->keyLadderSelect != BCMD_KeyLadderSelection_eReserved2 )
    {
        hHsm->vkl[BHSM_RemapVklId (pGrk->virtualKeyLadderID )].custSubMode = pGrk->customerSubMode;
        hHsm->vkl[BHSM_RemapVklId (pGrk->virtualKeyLadderID )].client      = pGrk->client;
    }

BHSM_P_DONE_LABEL:

    if( hMsg != NULL )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE(BHSM_GenerateRouteKey);
    return( errCode );
}


#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
BERR_Code  BHSM_GenerateGlobalKey( BHSM_Handle hHsm, BHSM_GenerateGlobalKey_t *pGlobalKey )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             status;
    unsigned            keyLength = 0;

    BDBG_ENTER(BHSM_GenerateGlobalKey);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pGlobalKey == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) )!= BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    header.commandLength = 208+(4*5);
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY, &header );

    switch( pGlobalKey->keySize )
    {
        case BCMD_KeySize_e64:  keyLength = ( 64/8); break;
        case BCMD_KeySize_e128: keyLength = (128/8); break;
        case BCMD_KeySize_e192: keyLength = (192/8); break;
        case BCMD_KeySize_e256: keyLength = (256/8); break;
        default: rc = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderSelection, BCMD_eFWKL );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderType, pGlobalKey->keyLadderType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eRootKeySrc, BCMD_RootKeySrc_eASKMGlobalKey );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eCustomerSel, BCMD_CustomerSubMode_eGeneralPurpose1 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eASKMSel, 1 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eNoKeyGenFlag, BCMD_KeyGenFlag_eGen );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eOwnerIDSelect, pGlobalKey->globalKeyOwnerId ); /*8*/
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eASKMGlobalKeyIndex, pGlobalKey->globalKeyIndex ); /*16*/

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLID, BHSM_RemapVklId(pGlobalKey->virtualKeyLadderId ));

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLayer, BCMD_KeyRamBuf_eKey3 );
    BHSM_BspMsg_PackArray( hMsg,
                           (BHSM_GEN_ROUTE_KEY_DATA_LEN - keyLength) + BCMD_GenKey_InCmd_eProcIn,
                           pGlobalKey->keyData,
                           keyLength );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeySize, pGlobalKey->keySize );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderOpera, pGlobalKey->operation );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eRouteKeyFlag, pGlobalKey->routeKeyRequired );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eSTBOwnerIDSel, pGlobalKey->stbOwnerId );
    BHSM_BspMsg_Pack16( hMsg, BCMD_GenKey_InCmd_eCAVendorID, pGlobalKey->caVendorId );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eTestKeySel, pGlobalKey->maskKeySelect );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eModuleID, pGlobalKey->moduleId );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_GenerateGlobalKey );

    return rc;
}
#endif


BERR_Code  BHSM_LoadVklOwnershipFromBsp ( BHSM_Handle hHsm )
{
    BERR_Code              rc = BERR_SUCCESS;
    uint32_t               i;
    uint8_t                association = 0;
    BHSM_BspMsg_h          hMsg = NULL;
    BHSM_BspMsgHeader_t    header;
    uint8_t                status  = 0;
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    uint8_t                allocation = 0;
    #endif

    BDBG_ENTER( BHSM_LoadVklOwnershipFromBsp );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLAssociationQuery, BCMD_VKLAssociationQueryFlag_eQuery );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderType, BCMD_KeyLadderType_e3DESABA );

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eNoKeyGenFlag, BCMD_KeyGenFlag_eNoGen );
    #endif

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLID, BCMD_VKL0 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLayer, BCMD_KeyRamBuf_eKey3 );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS )
    {
        BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

    for(i = 0; i < BHSM_MAX_VKL; i++ ) /* store the just retrieved VKL-CustSubMode association table */
    {
        /*    Store existing Customer SubMode association */
        BHSM_BspMsg_Get8( hMsg, BCMD_GenKey_OutCmd_eVKLAssociation+(i*4), &association );
        hHsm->vkl[i].custSubMode = association;

        hHsm->vkl[i].free = true;  /* every VKL is available */

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)

        if( association == 0xFF )
        {
            hHsm->vkl[i].client = BHSM_ClientType_eNone;
            hHsm->vkl[i].neverUsed = true;

            BDBG_MSG(("VKL [%d]  None", i ));
        }
        else
        {
            BHSM_BspMsg_Get8( hMsg, BCMD_GenKey_OutCmd_eVKLAllocation+(i*4), &allocation ); /* sage v host allocation. */

            if( allocation != BHSM_ClientType_eHost &&
                allocation != BHSM_ClientType_eSAGE  )
            {
                rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR  ); /* out of spec parameter received from BSP. */
                goto BHSM_P_DONE_LABEL;
            }
            hHsm->vkl[i].client = allocation; /* allocation will be either _Host or _SAGE */
            hHsm->vkl[i].neverUsed = false;

            BDBG_MSG(("VKL [%d]  [%s]", i, (allocation == BHSM_ClientType_eHost ? "Host":"Sage") ));
        }
    #else
        /*    For Zeus 2.x and under, a VKL, once associated, becomes unavailable for association with other custSubModes */
        if( association == 0xFF )
        {
            hHsm->vkl[i].neverUsed = true;
        }
        else
        {
            hHsm->vkl[i].neverUsed = false;
        }
        BDBG_MSG(("VKL [%d]  %s free", i, ( hHsm->vkl[i].free ? "":"Not") ));
    #endif
    }

BHSM_P_DONE_LABEL:

    if( hMsg != NULL )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_LoadVklOwnershipFromBsp );
    return rc;
}

BERR_Code BHSM_ProcOutCmd (
        BHSM_Handle            hHsm,
        BHSM_ProcOutCmdIO_t    *pProcOutCmd
)
{

#if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,1)
    BERR_Code              rc = BERR_SUCCESS;
    BHSM_BspMsg_h          hMsg = NULL;
    BHSM_BspMsgHeader_t    header;
    uint8_t                status  = 0;
    uint16_t               responseLength  = 0;

    BDBG_ENTER( BHSM_ProcOutCmd );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pProcOutCmd == NULL )
    {
        return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( pProcOutCmd->unProcInLen > BHSM_MAX_PROC_IN_DATA_LEN )
    {
        return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_PROC_OUT_CMD, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_ProcOut_InCmd_eVKLID, BHSM_RemapVklId( pProcOutCmd->virtualKeyLadderID ));

    BHSM_BspMsg_PackArray( hMsg, BCMD_ProcOut_InCmd_eProcIn, &pProcOutCmd->aucProcIn[0], pProcOutCmd->unProcInLen );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pProcOutCmd->unStatus = (uint32_t)status;
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get16( hMsg, BCMD_CommonBufferFields_eParamLen, &responseLength );

    pProcOutCmd->unProcOutLen = responseLength - 4/*minus status length*/;

    if( pProcOutCmd->unProcOutLen > BHSM_MAX_PROC_OUT_DATA_LEN )
    {
        rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
        goto BHSM_P_DONE_LABEL;
    }

    #define OFFSET_TO_ProcOut (6) /* missing from BSP headerfiles.  */
    BHSM_BspMsg_GetArray( hMsg, OFFSET_TO_ProcOut, &pProcOutCmd->aucProcOut[0], pProcOutCmd->unProcOutLen );

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_ProcOutCmd );
    return rc;

#endif

    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pProcOutCmd );

    return  BERR_TRACE( BHSM_NOT_SUPPORTED_ERR );
}



BERR_Code BHSM_FastLoadEncryptedHdcpKey (
    BHSM_Handle                        hHsm,
    uint32_t                        index,
    BHSM_EncryptedHdcpKeyStruct     *keyStruct
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BHSM_GenerateRouteKeyIO_t        generateRouteKeyIO;
    uint32_t KeyParameter ;
    BCMD_RootKeySrc_e RootKeySrc;
    BHSM_AllocateVKLIO_t vkl;

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( keyStruct == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    /* allocate a VKL */
    BKNI_Memset( &vkl, 0, sizeof( vkl ) );
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    vkl.client = BHSM_ClientType_eHost;
   #else
    vkl.customerSubMode = BCMD_CustomerSubMode_eGeneralPurpose1;
   #endif

    if( ( errCode = BHSM_AllocateVKL( hHsm, &vkl ) ) != BERR_SUCCESS )
    {
        /* failed to allocate a keyladder. */
        return BERR_TRACE( errCode );
    }

    BKNI_Memset( &generateRouteKeyIO, 0, sizeof(BHSM_GenerateRouteKeyIO_t))  ;

    /* generate Key3 from custom key */
    generateRouteKeyIO.bASKMModeEnabled = 0;

    generateRouteKeyIO.subCmdID   = BCMD_VKLAssociationQueryFlag_eNoQuery;

    if (((keyStruct->Alg) & BHSM_KEYLADDER_HDCP_ROOT_KEY_SOURCE_MASK) == 0)
    {
        RootKeySrc = BCMD_RootKeySrc_eCusKey;
    }
    else
    {
        /* Root key is OTP key*/
        switch (keyStruct->CaDataLo)
        {
        case 1:
            RootKeySrc = BCMD_RootKeySrc_eOTPKeya;
            break;
        case 2:
            RootKeySrc = BCMD_RootKeySrc_eOTPKeyb;
            break;
        case 3:
            RootKeySrc = BCMD_RootKeySrc_eOTPKeyc;
            break;
        case 4:
            RootKeySrc = BCMD_RootKeySrc_eOTPKeyd;
            break;
        case 5:
            RootKeySrc = BCMD_RootKeySrc_eOTPKeye;
            break;
        case 6:
            RootKeySrc = BCMD_RootKeySrc_eOTPKeyf;
            break;
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        case 7:
            RootKeySrc = BCMD_RootKeySrc_eOTPKeyg;
            break;
        case 8:
            RootKeySrc = BCMD_RootKeySrc_eOTPKeyh;
            break;
#endif
        default:
            BHSM_FreeVKL( hHsm, vkl.allocVKL );
            return  BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
        }
    }

    generateRouteKeyIO.rootKeySrc = RootKeySrc;
    generateRouteKeyIO.customerSubMode = BCMD_CustomerSubMode_eGeneralPurpose1;

    generateRouteKeyIO.ucSwizzle1Index = 0;
    generateRouteKeyIO.swizzleType = (RootKeySrc == BCMD_RootKeySrc_eCusKey) ? BCMD_SwizzleType_eSwizzle0 : BCMD_SwizzleType_eNoSwizzle;

    generateRouteKeyIO.bUseCustKeyHighDecrypt = false ;
    generateRouteKeyIO.bUseCustKeyLowDecrypt =    false ;

    generateRouteKeyIO.virtualKeyLadderID = vkl.allocVKL;
    generateRouteKeyIO.keyLayer = BCMD_KeyRamBuf_eKey3 ;

    generateRouteKeyIO.key3Op                   = BCMD_Key3Op_eKey3NoProcess;
    generateRouteKeyIO.keyTweak                   = BHSM_KeyTweak_eNoTweak;
    generateRouteKeyIO.bIsKeyLadder3DESEncrypt = false;
    generateRouteKeyIO.bSwapAesKey               = false;

    generateRouteKeyIO.keyLadderType = BCMD_KeyLadderType_e3DESABA;
    generateRouteKeyIO.keySize= BCMD_KeySize_e64;
    generateRouteKeyIO.ucKeyDataLen = 8 ;

    generateRouteKeyIO.bIsRouteKeyRequired = true;
    generateRouteKeyIO.keyDestBlckType = BCMD_KeyDestBlockType_eHdmi;
    generateRouteKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey;
    generateRouteKeyIO.keyDestIVType           = BCMD_KeyDestIVType_eNoIV;
    generateRouteKeyIO.caKeySlotType = BCMD_XptSecKeySlot_eType0 ;
    generateRouteKeyIO.keyMode = BCMD_KeyMode_eRegular;


    generateRouteKeyIO.keyLadderType = (keyStruct->Alg) & (~BHSM_KEYLADDER_HDCP_ROOT_KEY_SOURCE_MASK);
    generateRouteKeyIO.ucCustKeyLow = keyStruct->cusKeySel;
    generateRouteKeyIO.ucKeyVarLow = keyStruct->cusKeyVarL;
    generateRouteKeyIO.ucCustKeyHigh = keyStruct->cusKeySel;
    generateRouteKeyIO.ucKeyVarHigh = keyStruct->cusKeyVarH;

    BKNI_Memset(&generateRouteKeyIO.aucKeyData, 0, sizeof(generateRouteKeyIO.aucKeyData));
    KeyParameter = keyStruct->HdcpKeyHi;
    generateRouteKeyIO.aucKeyData[0]  =  (KeyParameter >> 24) & 0xff ;
    generateRouteKeyIO.aucKeyData[1]  =  (KeyParameter >> 16) & 0xff ;
    generateRouteKeyIO.aucKeyData[2]  =  (KeyParameter >>    8) & 0xff ;
    generateRouteKeyIO.aucKeyData[3]  =  (KeyParameter)           & 0xff ;

    KeyParameter = keyStruct->HdcpKeyLo;
    generateRouteKeyIO.aucKeyData[4]  =  (KeyParameter >> 24) & 0xff ;
    generateRouteKeyIO.aucKeyData[5]  =  (KeyParameter >> 16) & 0xff ;
    generateRouteKeyIO.aucKeyData[6]  =  (KeyParameter >>  8) & 0xff ;
    generateRouteKeyIO.aucKeyData[7]  =  (KeyParameter)           & 0xff ;

    generateRouteKeyIO.unKeySlotNum = index ;
    errCode= BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO) ;
    BDBG_MSG(("generateRouteKeyIO key 3 unStatus = 0x%08X", generateRouteKeyIO.unStatus)) ;

    BHSM_FreeVKL( hHsm, vkl.allocVKL );

    return( errCode );
}

BERR_Code BHSM_KladChallenge (
    BHSM_Handle              hHsm,
    BHSM_KladChallengeIO_t   *pChallenge
)
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)

    BERR_Code errCode = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;

    BDBG_ENTER(BHSM_KladChallenge);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pChallenge == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    errCode = BHSM_BspMsg_Create( hHsm, &hMsg );
    if (errCode != BERR_SUCCESS) {
        BERR_TRACE(errCode);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader(&header);
    BHSM_BspMsg_Header(hMsg, BCMD_cmdType_eKLADChallengeCmd, &header);
    header.continualMode = CONT_MODE_ALL_DATA;

    BHSM_BspMsg_Pack8(hMsg, BCMD_KLAD_Challenge_InputCommandField_eId, pChallenge->keyId);

    errCode = BHSM_BspMsg_SubmitCommand(hMsg);
    if (errCode != BERR_SUCCESS) {
        errCode = BERR_TRACE(errCode);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8(hMsg, BCMD_KLAD_Challenge_OutCmdField_eStatus, &status);
    pChallenge->unStatus = (uint32_t)status;
    if (pChallenge->unStatus != 0)
    {
        errCode = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

    /* retrieve KeyId */
    BHSM_BspMsg_GetArray(hMsg, BCMD_KLAD_Challenge_OutCmdField_eIdLo, &pChallenge->aucKeyId[0], 4);
    BHSM_BspMsg_GetArray(hMsg, BCMD_KLAD_Challenge_OutCmdField_eIdHi, &pChallenge->aucKeyId[4], 4);

    /* retrieve BlackBoxID and STBOwnerID, only available in ASKM mode */
    BHSM_BspMsg_Get8(hMsg, BCMD_KLAD_Challenge_OutCmdField_eBlackBoxID, &pChallenge->ucBlackBoxID);

    /*
    BHSM_BspMsg_Get32(hMsg, BCMD_KLAD_Challenge_OutCmdField_eSTBOwnerID, &pChallenge->STBOwnerIDMsp);
    pChallenge->STBOwnerIDMsp &= 0x0000FFFF;
    */

BHSM_P_DONE_LABEL:

    if (hMsg != NULL) {
        (void)BHSM_BspMsg_Destroy(hMsg);
    }

    BDBG_LEAVE(BHSM_KladChallenge);
    return errCode;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pChallenge );
    return  BERR_TRACE( BHSM_NOT_SUPPORTED_ERR );
#endif
}


BERR_Code BHSM_KladResponse (
    BHSM_Handle             hHsm,
    BHSM_KladResponseIO_t  *pResponse
)
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    BERR_Code errCode = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;

    BDBG_ENTER(BHSM_KladResponse);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pResponse == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    errCode = BHSM_BspMsg_Create( hHsm, &hMsg );
    if (errCode != BERR_SUCCESS) {
        BERR_TRACE(errCode);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader(&header);
    BHSM_BspMsg_Header(hMsg, BCMD_cmdType_eKLADResponseCmd, &header);
    header.continualMode = CONT_MODE_ALL_DATA;

    BHSM_BspMsg_Pack8(hMsg, BCMD_KLAD_Response_InCmdField_eKLADMode, pResponse->kladMode);

    BHSM_BspMsg_Pack8(hMsg, BCMD_KLAD_Response_InCmd_eVKLID, BHSM_RemapVklId(pResponse->virtualKeyLadderID));

    BHSM_BspMsg_Pack8(hMsg, BCMD_KLAD_Response_InCmd_eKeyLayer, pResponse->keyLayer);

    /* Load Nonce - 128 bit */
    BHSM_BspMsg_PackArray(hMsg,
                          BCMD_KLAD_Response_InCmdField_eNonce,
                          pResponse->aucNonce,
                          BHSM_MAX_NONCE_DATA_LEN);

    errCode = BHSM_BspMsg_SubmitCommand(hMsg);
    if (errCode != BERR_SUCCESS) {
        errCode = BERR_TRACE(errCode);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8(hMsg, BCMD_KLAD_Response_OutCmdField_eStatus, &status);
    pResponse->unStatus = (uint32_t)status;
    if (pResponse->unStatus != 0)
    {
        errCode = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetArray(hMsg,
                         BCMD_KLAD_Response_OutCmdField_eResponse,
                         pResponse->aucReponse,
                         BHSM_MAX_NONCE_DATA_LEN);

BHSM_P_DONE_LABEL:

    if (hMsg != NULL) {
        (void)BHSM_BspMsg_Destroy(hMsg);
    }

    BDBG_LEAVE(BHSM_KladResponse);
    return errCode;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pResponse );
    return  BERR_TRACE( BHSM_NOT_SUPPORTED_ERR );
#endif
}
