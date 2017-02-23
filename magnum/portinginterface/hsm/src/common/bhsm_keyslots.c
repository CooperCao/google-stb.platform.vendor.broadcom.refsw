/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhsm_keyslots_private.h"
#include "bhsm_keyladder.h"
#include "bhsm_bsp_msg.h"
#include "../keyladder/bhsm_keyladder_enc_private.h"
#include "bchp_bsp_control_intr2.h"
#include "bchp_bsp_glb_control.h"
#if BHSM_HOST_SAGE_SRAM_INTERFACE
#include "bchp_scpu_globalram.h"
#endif

#if BHSM_SUPPORT_DEBUG_READ_OTP_TYPE
#include "bhsm_otp_configuration.h"
#endif


BDBG_MODULE(BHSM);

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
 #define BHSM_BYPASS_KEYSLOT_TYPE     ( BCMD_XptSecKeySlot_eType0 )
 #define BHSM_BYPASS_KEYSLOT_NUMBER_g2gr ( BHSM_BypassKeySlot_eG2GR )
 #define BHSM_BYPASS_KEYSLOT_NUMBER_gr2r ( BHSM_BypassKeySlot_eGR2R)
 #define BHSM_BYPASS_KEYSLOT_BLOCK    ( BCMD_KeyDestBlockType_eCA )
 #define BHSM_BYPASS_KEYSLOT_POLARITY ( BCMD_KeyDestEntryType_eOddKey )
#endif

#if BHSM_HOST_SAGE_SRAM_INTERFACE
/* Available range is 0x50-0x5F  */
#define BHSM_STASH_OFFSET                    (0x50*4)
#define BHSM_STASH_MAX_SIZE                  (0xF*4)
#define BHSM_STASH_OFFSET_SIGNATURE          (BHSM_STASH_OFFSET)
#define BHSM_STASH_OFFSET_M2M_SLOTS          (BHSM_STASH_OFFSET_SIGNATURE         + (BHSM_STASH_DWORDSIZE_SIGNATURE*4))
#define BHSM_STASH_OFFSET_NUM_KEYSLOT_TYPES  (BHSM_STASH_OFFSET_M2M_SLOTS         + (BHSM_STASH_DWORDSIZE_M2M_SLOTS*4))
#define BHSM_STASH_OFFSET_KEYSLOT_TYPES      (BHSM_STASH_OFFSET_NUM_KEYSLOT_TYPES + (BHSM_STASH_DWORDSIZE_NUM_KEYSLOT_TYPES*4) )
#endif

#define BHSM_BSP_FW_VERSION_KEYSLOT_MULTIPLE_PID_CHANNELS (4)

BERR_Code  BHSM_InitialiseKeyLadders ( BHSM_Handle hHsm );

static BERR_Code ConfigPidKeyPointerTable ( BHSM_Handle hHsm, BHSM_ConfigPidKeyPointerTableIO_t *pPidChannelConf, bool verbose );

#if BHSM_SUPPORT_KEYSLOT_OWNERSHIP
/* checks a keylsot is belonging t CA provider Zzyzx */
static bool isKeySlotTypeZzyzx( BCMD_XptSecKeySlot_e keySlotType );
#endif

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
/* Initialise PID Channels */
static BERR_Code InitialisePidChannels( BHSM_Handle hHsm );
#endif



#define BHSM_KEYSLOT_WORD_COUNT ((BCMD_XptSecKeySlot_eTypeMax+3)/4)  /* size rounded up */

/* Stash keyslot parameters to SRAM so that the can be retrieved (even from SAGE context.) */
void BHSM_P_StashKeySlotTableWrite( BHSM_Handle hHsm, BHSM_KeyslotTypes_t *pKeyslots )
{
#if BHSM_HOST_SAGE_SRAM_INTERFACE
    unsigned    stashAddress = 0;
    unsigned    stashStartAddress = 0;
    unsigned    i;
    uint32_t    stash[BHSM_KEYSLOT_WORD_COUNT] = {0};
    unsigned    wordOffset = 0;
    unsigned    byteOffset;
    uint32_t    numSlots;

    BDBG_ENTER( BHSM_P_StashKeySlotTableWrite );

    if( (BHSM_ZEUS_VERSION  > BHSM_ZEUS_VERSION_CALC(4,2)) ||
       ((BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2)) && isBfwVersion_GreaterOrEqual( hHsm, 4, 1, 0 )) )
    {
        return;  /* We are quaranteed to be coupled with SAGE3+. SAGE will refer to BSP for this information. */
    }

    if( pKeyslots->numKeySlotTypes > BCMD_XptSecKeySlot_eTypeMax )
    {
        BDBG_ERR(("#[%d]slot types too big [%d][%d]", __LINE__, pKeyslots->numKeySlotTypes, BCMD_XptSecKeySlot_eTypeMax ));
        return;
    }

    stashStartAddress = BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BHSM_STASH_OFFSET_NUM_KEYSLOT_TYPES;
    stashAddress = stashStartAddress;

    BREG_Write32( hHsm->regHandle, stashAddress, pKeyslots->numKeySlotTypes );

    BDBG_MSG(("STASH KeySlot[p%08X] #types[%0d]", stashAddress, pKeyslots->numKeySlotTypes ));

    for( i = 0; i < pKeyslots->numKeySlotTypes; i++ )
    {
        numSlots = (pKeyslots->numKeySlot[i]) & 0xFF;  /* can only occupy one byte */

        wordOffset = i/4;
        byteOffset = i%4;

        stash[wordOffset] |= (numSlots << (byteOffset*8));

        BDBG_MSG(("STASH KeySlot[%d][%02X]", i, numSlots ));
    }

    stashAddress = BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BHSM_STASH_OFFSET_KEYSLOT_TYPES;
    for( i = 0; i < BHSM_KEYSLOT_WORD_COUNT; i++ )
    {
        if( stashAddress - stashStartAddress > (BHSM_STASH_MAX_SIZE-4) )
        {
            BDBG_ASSERT( !"ATTEMPTING TO EXCEED AVAILABLE SRAM" ); /* it's a static problem. */
            return;
        }
        BREG_Write32( hHsm->regHandle, stashAddress, stash[i] );
        BDBG_MSG(("STASH Write KeySlot[%08X] [p%08X]", stash[i],  stashAddress ));
        stashAddress += 4;
    }

    BDBG_LEAVE( BHSM_P_StashKeySlotTableWrite );

    return;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pKeyslots );
    (void)BERR_TRACE( BERR_NOT_SUPPORTED );
    return;
#endif /* #if (BHSM_HOST_SAGE_SRAM_INTERFACE ) */
}

/* Read keyslot parameters from SRAM. */
BERR_Code BHSM_P_StashKeySlotTableRead( BHSM_Handle hHsm, BHSM_KeyslotTypes_t *pKeyslots )
{
#if BHSM_HOST_SAGE_SRAM_INTERFACE
    unsigned    stashAddress = 0;
    unsigned    i;
    uint32_t    stash[BHSM_KEYSLOT_WORD_COUNT] = {0};
    unsigned    wordOffset = 0;
    unsigned    byteOffset;

    BDBG_ENTER( BHSM_P_StashKeySlotTableRead );

    stashAddress = BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BHSM_STASH_OFFSET_NUM_KEYSLOT_TYPES;
    pKeyslots->numKeySlotTypes = BREG_Read32(hHsm->regHandle, stashAddress );

    if( pKeyslots->numKeySlotTypes > BCMD_XptSecKeySlot_eTypeMax )
    {
        BDBG_ERR(("#[%d]slot types too big [%d][%d]", __LINE__, pKeyslots->numKeySlotTypes, BCMD_XptSecKeySlot_eTypeMax ));
        BKNI_Memset(pKeyslots, 0, sizeof(*pKeyslots));
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    stashAddress = BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BHSM_STASH_OFFSET_KEYSLOT_TYPES;
    for( i = 0; i < BHSM_KEYSLOT_WORD_COUNT; i++ )
    {
        stash[i] = BREG_Read32(hHsm->regHandle, stashAddress );

        BDBG_MSG(("STASH Read KeySlot[%08X] [p%08X]", stash[i],  stashAddress ));

        stashAddress+=4;
    }

    for( i = 0; i < pKeyslots->numKeySlotTypes; i++ )
    {
        wordOffset = i/4;
        byteOffset = i%4;

        pKeyslots->numKeySlot[i] = (stash[wordOffset] >> (byteOffset*8)) & 0xFF;

        BDBG_MSG(("UNSTASH KeySlot[%d][%08X][%02X]", i, stash[wordOffset], pKeyslots->numKeySlot[i] ));
    }

    BDBG_LEAVE( BHSM_P_StashKeySlotTableRead );
    return BERR_SUCCESS;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pKeyslots );
    return BERR_TRACE (BERR_NOT_SUPPORTED);
#endif /* #if (BHSM_HOST_SAGE_SRAM_INTERFACE ) */
}


BERR_Code BHSM_P_BspKeySlotTableRead ( BHSM_Handle hHsm, BHSM_KeyslotTypes_t * pKeyslotsInfo )
{
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2))
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_KeySlotsStatus_t*  pKeySlotsStatus;
    BCMD_XptSecKeySlot_e    keySlotType;
    uint16_t                slotNumOffset = 0;

    BDBG_ENTER ( BHSM_P_BspKeySlotTableRead );

    if ( hHsm == NULL || pKeyslotsInfo == NULL )
    {
        return BERR_TRACE ( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( (BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2)) && !isBfwVersion_GreaterOrEqual( hHsm, 4,0,1 ) )
    {
        return BERR_TRACE (BERR_NOT_SUPPORTED); /* Information is available from the BSP. */
    }

    pKeySlotsStatus = (BHSM_KeySlotsStatus_t*) BKNI_Malloc( sizeof(BHSM_KeySlotsStatus_t) );
    if( pKeySlotsStatus == NULL)
    {
        BDBG_ERR(( "Failed to allocate memory for KeySlotStatus [%d]", (unsigned)sizeof(pKeySlotsStatus) ));
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    BKNI_Memset( pKeySlotsStatus, 0, sizeof(BHSM_KeySlotsStatus_t) );

    if ( ( rc = BHSM_QueryKeySlotsStatus ( hHsm, pKeySlotsStatus ) ) != BERR_SUCCESS )
    {
        BERR_TRACE ( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BKNI_Memset ( pKeyslotsInfo, 0, sizeof ( BHSM_KeyslotTypes_t ) );

    /* The key slot status information is expected as the following byte order. */

    /* |type0 slot0, type0 slot1, type0 slot2 ... | */
    /* |type1 slot0, type1 slot1, type1 slot2 ... | */
    /* | ...                                      | */
    /* |type5 slot0, type5 slot1, type5 slot2 ... | */

    slotNumOffset = 0;

    for ( keySlotType = BCMD_XptSecKeySlot_eType0; keySlotType <= BHSM_KeySlotStatusQuery_eTypeMax; keySlotType++ )
    {
        if ( pKeySlotsStatus->slot[slotNumOffset].type == keySlotType )
        {
            /* The final value will be returned. */

            pKeyslotsInfo->numKeySlot[keySlotType] = pKeySlotsStatus->slot[slotNumOffset].number;

            if ( pKeyslotsInfo->numKeySlot[keySlotType] )
            {
                pKeyslotsInfo->numKeySlotTypes = keySlotType + 1;
            }

            /* To be updated to real value when BFW has it. */
            pKeyslotsInfo->numMulti2KeySlots = 0;

            /* offset of the next keyslot type */
            slotNumOffset += pKeySlotsStatus->slot[slotNumOffset].number;
        }
    }

BHSM_P_DONE_LABEL:

    BKNI_Free( pKeySlotsStatus );
    BDBG_LEAVE ( BHSM_P_BspKeySlotTableRead );

    return rc;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pKeyslotsInfo );
    return BERR_TRACE (BERR_NOT_SUPPORTED);
#endif /* #if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)) */
}

BERR_Code BHSM_P_KeySlotsTableConfGet( BHSM_Handle hHsm, BHSM_KeyslotTypes_t *pKeyslots )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER ( BHSM_P_KeySlotsTableConfGet );

    if( !pKeyslots )
    {
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    BKNI_Memset( pKeyslots, 0, sizeof(*pKeyslots) );

    if( (BHSM_ZEUS_VERSION  > BHSM_ZEUS_VERSION_CALC(4,2)) ||
       ((BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2)) && isBfwVersion_GreaterOrEqual( hHsm, 4, 0, 1 )) )
    {
        if( (rc = BHSM_P_BspKeySlotTableRead( hHsm, pKeyslots )) != BERR_SUCCESS )
        {
            return BERR_TRACE( rc );
        }
    }
    else
    {
        if( (rc = BHSM_P_StashKeySlotTableRead( hHsm, pKeyslots )) != BERR_SUCCESS )
        {
            return BERR_TRACE( rc );
        }
    }

    BDBG_LEAVE ( BHSM_P_KeySlotsTableConfGet );
    return rc;
}


#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
/*
    Allocate the two bypass keyslots, g2gr + gr2r .
     -- needs to be called on system initialisation.
*/
static BERR_Code AllocateByPassKeyslots( BHSM_Handle hHsm )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_XptKeySlotIO_t keySlotConfig;

    BDBG_ENTER( AllocateByPassKeyslots );

    BKNI_Memset( &keySlotConfig, 0, sizeof(keySlotConfig) );
    keySlotConfig.client      = BHSM_ClientType_eHost;
    keySlotConfig.keySlotType = BHSM_BYPASS_KEYSLOT_TYPE;
    if( (rc = BHSM_AllocateCAKeySlot ( hHsm, &keySlotConfig ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    if( keySlotConfig.keySlotNum != BHSM_BYPASS_KEYSLOT_NUMBER_g2gr )
    {
        /* Expect first allocated KeySlot to be equal to BHSM_BYPASS_KEYSLOT_NUMBER_g2gr */
        rc = BERR_TRACE( BHSM_STATUS_FAILED );
        goto BHSM_P_DONE_LABEL;
    }

    if( (rc = BHSM_AllocateCAKeySlot ( hHsm, &keySlotConfig ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }
    if( keySlotConfig.keySlotNum != BHSM_BYPASS_KEYSLOT_NUMBER_gr2r )
    {
        /* Expect second allocated KeySlot to be equal to BHSM_BYPASS_KEYSLOT_NUMBER_gr2r */
        rc = BERR_TRACE( BHSM_STATUS_FAILED );
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    BDBG_LEAVE( AllocateByPassKeyslots );

    return rc;
}

#endif /* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1) */

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
/* Configure both the g2gr and gr2r bypass keyslots. */
BERR_Code BHSM_InitialiseBypassKeyslots( BHSM_Handle hHsm )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_InvalidateKeyIO_t invalidateKeySlot;
    BHSM_KeySlotOwnership_t  keyslotOwnership;

    BDBG_ENTER( BHSM_InitialiseBypassKeyslots );

    BKNI_Memset( &keyslotOwnership, 0, sizeof(keyslotOwnership) );
    keyslotOwnership.keySlotNumber = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    keyslotOwnership.keySlotType   = BHSM_BYPASS_KEYSLOT_TYPE;
    if( (rc = BHSM_GetKeySlotOwnership(hHsm, &keyslotOwnership) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    if(keyslotOwnership.owner == BHSM_KeySlotOwner_eSAGE)
    {
        /*Sage has configured, leave the bypass keyslots alone*/
        rc = BERR_SUCCESS;
        goto BHSM_P_DONE_LABEL;
    }

    /* Configure first bypass keyslot ... g2gr */
    BKNI_Memset( &invalidateKeySlot, 0, sizeof(invalidateKeySlot) );
    invalidateKeySlot.caKeySlotType         = BHSM_BYPASS_KEYSLOT_TYPE;
    invalidateKeySlot.unKeySlotNum          = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    invalidateKeySlot.invalidKeyType        = BCMD_InvalidateKey_Flag_eDestKeyOnly;
    invalidateKeySlot.bInvalidateAllEntries = true;
    invalidateKeySlot.bypassConfiguraion    = BCMD_ByPassKTS_eFromGToRG;
    invalidateKeySlot.virtualKeyLadderID = BCMD_VKL_KeyRam_eMax; /*Not used. Setting to invalid value to avoid collision with VKL in use*/
    if( ( rc = BHSM_InvalidateKey( hHsm, &invalidateKeySlot ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc ); /* failed to invalidate the g2gr bypass keyslot */
        goto BHSM_P_DONE_LABEL;
    }

    /* Configure second bypass keyslot. */
    invalidateKeySlot.unKeySlotNum = BHSM_BYPASS_KEYSLOT_NUMBER_gr2r;

    /* If on sage, or on host and sage is enabled. */
    if(   hHsm->currentSettings.clientType == BHSM_ClientType_eSAGE ||
        ( hHsm->currentSettings.clientType == BHSM_ClientType_eHost &&  hHsm->currentSettings.sageEnabled == true ) )
    {
        invalidateKeySlot.bypassConfiguraion = BCMD_ByPassKTS_eFromRGToR;
    }

    if( ( rc = BHSM_InvalidateKey( hHsm, &invalidateKeySlot ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc ); /* failed to invalidate the second bypass keyslot */
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    BDBG_LEAVE( BHSM_InitialiseBypassKeyslots );
    return rc;
}

/* Configure both the g2gr and gr2r bypass keyslots. */
BERR_Code BHSM_InitialiseBypassKeyslots_sage( BHSM_Handle hHsm )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_InvalidateKeyIO_t  resetKeySlot;
    BHSM_ConfigAlgorithmIO_t algorithmConfig;
    BHSM_LoadRouteUserKeyIO_t userKey;
    BHSM_KeySlotGlobalCntrlWord_t keySlotControlWord;

    BDBG_ENTER( BHSM_InitialiseBypassKeyslots_sage );

    if( hHsm->currentSettings.clientType != BHSM_ClientType_eSAGE )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED ); /* Function can only be called from SAGE context. */
    }

    /* Invalidate both bypass filters keyslot. */
    BKNI_Memset( &resetKeySlot, 0, sizeof(resetKeySlot) );
    resetKeySlot.caKeySlotType      = BHSM_BYPASS_KEYSLOT_TYPE;
    resetKeySlot.unKeySlotNum       = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    resetKeySlot.keyDestBlckType    = BCMD_KeyDestBlockType_eCPScrambler;
    resetKeySlot.invalidKeyType     = BCMD_InvalidateKey_Flag_eDestKeyOnly;
    resetKeySlot.keyDestEntryType   = BCMD_KeyDestEntryType_eOddKey;
    resetKeySlot.virtualKeyLadderID = 0; /* Not used ... BCMD_InvalidateKey_Flag_eDestKeyOnly */
    resetKeySlot.bInvalidateAllEntries = true;
    if( ( rc = BHSM_InvalidateKey( hHsm, &resetKeySlot ) ) != BERR_SUCCESS )
    {
        BDBG_ERR(("%s BHSM_InvalidateKey for GRG keyslot failed\n", __FUNCTION__ ));
        goto BHSM_P_DONE_LABEL;
    }

    resetKeySlot.unKeySlotNum = BHSM_BYPASS_KEYSLOT_NUMBER_gr2r;
    if( ( rc = BHSM_InvalidateKey( hHsm, &resetKeySlot ) ) != BERR_SUCCESS )
    {
        BDBG_ERR(("%s BHSM_InvalidateKey for RG2R keyslot failed\n", __FUNCTION__ ));
        goto BHSM_P_DONE_LABEL;
    }

    /* Set G2RG keyslot globals */
    BKNI_Memset( &keySlotControlWord, 0, sizeof(keySlotControlWord) );
    keySlotControlWord.caKeySlotType     = BHSM_BYPASS_KEYSLOT_TYPE;
    keySlotControlWord.unKeySlotNum      = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    keySlotControlWord.encryptBeforeRAVE = 0;
    keySlotControlWord.inputRegion   = BHSM_REGION_GLR;
    keySlotControlWord.RpipeOutput   = BHSM_REGION_GLR | BHSM_REGION_CRR;
    keySlotControlWord.GpipeOutput   = BHSM_REGION_GLR | BHSM_REGION_CRR;

    if( ( rc = BHSM_ConfigKeySlotGlobalCntrlWord( hHsm, &keySlotControlWord ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    keySlotControlWord.inputRegion       = BHSM_REGION_CRR | BHSM_REGION_GLR;
    keySlotControlWord.RpipeOutput       = BHSM_REGION_CRR;
    keySlotControlWord.GpipeOutput       = BHSM_REGION_CRR;
    keySlotControlWord.unKeySlotNum = BHSM_BYPASS_KEYSLOT_NUMBER_gr2r;
    if( ( rc = BHSM_ConfigKeySlotGlobalCntrlWord( hHsm, &keySlotControlWord ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    /* Configure G2GR keyslot */
    BKNI_Memset( &algorithmConfig, 0, sizeof (algorithmConfig) );
    algorithmConfig.keyDestBlckType     = BHSM_BYPASS_KEYSLOT_BLOCK;
    algorithmConfig.keyDestEntryType    = BHSM_BYPASS_KEYSLOT_POLARITY;
    algorithmConfig.caKeySlotType       = BHSM_BYPASS_KEYSLOT_TYPE;
    algorithmConfig.unKeySlotNum        = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    algorithmConfig.cryptoAlg.cryptoOp  = BHSM_M2mAuthCtrl_ePassThrough;
    algorithmConfig.cryptoAlg.xptSecAlg          = BCMD_XptM2MSecCryptoAlg_eAes128;
    algorithmConfig.cryptoAlg.cipherDVBCSA2Mode  = BCMD_CipherModeSelect_eECB;
    algorithmConfig.cryptoAlg.termCounterMode    = BCMD_TerminationMode_eCLEAR;
    algorithmConfig.cryptoAlg.bUseExtKey   = false;
    algorithmConfig.cryptoAlg.bUseExtIV    = false;
    algorithmConfig.cryptoAlg.bGpipeEnable = false; /* bypass G */
    algorithmConfig.cryptoAlg.bRpipeEnable = false; /* bypass R */
    if( ( rc = BHSM_ConfigAlgorithm ( hHsm, &algorithmConfig ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    /* Configure RG2R keyslot  */
    algorithmConfig.unKeySlotNum = BHSM_BYPASS_KEYSLOT_NUMBER_gr2r;
    if( ( rc = BHSM_ConfigAlgorithm ( hHsm, &algorithmConfig ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    /* Route keyslot configuration G2GR */
    BKNI_Memset( &userKey, 0, sizeof(userKey) );
    userKey.bIsRouteKeyRequired = true;
    userKey.keyDestBlckType     = BHSM_BYPASS_KEYSLOT_BLOCK;
    userKey.keySize.eKeySize    = BCMD_KeySize_e128;
    userKey.bIsRouteKeyRequired = true;
    userKey.keyDestEntryType    = BHSM_BYPASS_KEYSLOT_POLARITY;
    userKey.caKeySlotType       = BHSM_BYPASS_KEYSLOT_TYPE;
    userKey.unKeySlotNum        = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    userKey.keyMode             = BCMD_KeyMode_eRegular;
    if( ( rc = BHSM_LoadRouteUserKey( hHsm, &userKey ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }
    /* Route keyslot configuration GR2R */
    userKey.unKeySlotNum = BHSM_BYPASS_KEYSLOT_NUMBER_gr2r;
    if( ( rc = BHSM_LoadRouteUserKey( hHsm, &userKey ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    BDBG_LEAVE( BHSM_InitialiseBypassKeyslots_sage );
    return rc;
}
#endif

/* initialise internal keyslot data structure. */
BERR_Code BHSM_P_KeySlotsInitialise( BHSM_Handle hHsm, BHSM_KeyslotTypes_t *pKeyslots )
{
    uint32_t  i;
    uint32_t  j;
    uint32_t  k;
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_CAKeySlotInfo_t *pKeyslot = NULL;

    BDBG_ENTER( BHSM_P_KeySlotsInitialise );

    BKNI_Memset( hHsm->keySlotTypes, 0, sizeof(hHsm->keySlotTypes));

    /* Keep track of the number of key slots per key slot type */

    if( pKeyslots->numKeySlotTypes > BCMD_XptSecKeySlot_eTypeMax )
    {
        BDBG_ERR(("#[%d]slot types too big [%d][%d]", __LINE__, pKeyslots->numKeySlotTypes, BCMD_XptSecKeySlot_eTypeMax ));
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( pKeyslots->numKeySlotTypes == 0 )
    {
        BDBG_ERR(("#[%d]slot types is 0 [%d]",  __LINE__, BCMD_XptSecKeySlot_eTypeMax ));
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    hHsm->numKeySlotTypes = pKeyslots->numKeySlotTypes;

    /*set the first manually. */
    hHsm->keySlotTypes[0].unKeySlotNum = pKeyslots->numKeySlot[0];
    hHsm->keySlotTypes[0].ucKeySlotStartOffset = 0;

    for( i = 1; i < pKeyslots->numKeySlotTypes; i++ )
    {
        hHsm->keySlotTypes[i].unKeySlotNum = pKeyslots->numKeySlot[i];
        hHsm->keySlotTypes[i].ucKeySlotStartOffset = hHsm->keySlotTypes[i-1].ucKeySlotStartOffset +
                                                     hHsm->keySlotTypes[i-1].unKeySlotNum;
    }
    hHsm->numMulti2KeySlots = pKeyslots->numMulti2KeySlots;

    hHsm->unTotalCAKeySlotNum = 0;
    for( i = 0; i < pKeyslots->numKeySlotTypes; i++ )
    {
        hHsm->unTotalCAKeySlotNum += pKeyslots->numKeySlot[i];

        if( hHsm->unTotalCAKeySlotNum > BHSM_MAX_KEYLSOTS )
        {
            /* inconsistency in data ... too many keyslots to initialise */
            return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
        }

        k = hHsm->keySlotTypes[i].ucKeySlotStartOffset;
        for ( j = 0; j < hHsm->keySlotTypes[i].unKeySlotNum; j++ )
        {
            pKeyslot = BKNI_Malloc( sizeof(*pKeyslot) );
            if( pKeyslot == NULL )
            {
                rc = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
                goto BHSM_P_DONE_LABEL;
            }

            BKNI_Memset( pKeyslot, 0, sizeof(*pKeyslot) );

            pKeyslot->keySlotType = i;
            pKeyslot->bIsUsed     = false;
            pKeyslot->client      = BHSM_ClientType_eNone; /* maybe updated below */

           #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
            {
                BHSM_KeySlotOwnership_t ownership;
                BKNI_Memset( &ownership, 0, sizeof(ownership) );
                ownership.keySlotNumber = j;
                ownership.keySlotType   = i;

                if( ( rc = BHSM_GetKeySlotOwnership( hHsm, &ownership ) ) !=  BERR_SUCCESS )
                {
                    (void)BERR_TRACE( rc );
                    BKNI_Free( pKeyslot );
                    pKeyslot = NULL;
                    goto BHSM_P_DONE_LABEL;
                }

                switch( ownership.owner )
                {
                    case BHSM_KeySlotOwner_eFREE:   { pKeyslot->client = BHSM_ClientType_eNone; break; }
                    case BHSM_KeySlotOwner_eSAGE:   { pKeyslot->client = BHSM_ClientType_eSAGE; break; }
                    case BHSM_KeySlotOwner_eSHARED: { pKeyslot->client = BHSM_ClientType_eHost; break; }
                    default:                        { BERR_TRACE( BHSM_STATUS_FAILED ); } /* just print an error. */
                }
            }
           #endif /* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1) */

           hHsm->pKeySlot[k + j] = pKeyslot;
           pKeyslot = NULL;
        }
    }

    BDBG_MSG(("NUM KeySlots Total [%d]", hHsm->unTotalCAKeySlotNum ));

BHSM_P_DONE_LABEL:

    BDBG_LEAVE( BHSM_P_KeySlotsInitialise );

    return rc;
}

#define  BHSM_BSP_INIT_KEYSLOT_eMulti2 ( BCMD_InitKeySlot_InCmdCfg_eConfigMulti2Slot )
#define  BHSM_BSP_INIT_KEYSLOT_eType0  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber       )
#define  BHSM_BSP_INIT_KEYSLOT_eType1  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(1<<2))
#define  BHSM_BSP_INIT_KEYSLOT_eType2  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(2<<2))
#define  BHSM_BSP_INIT_KEYSLOT_eType3  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(3<<2))
#define  BHSM_BSP_INIT_KEYSLOT_eType4  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(4<<2))
#define  BHSM_BSP_INIT_KEYSLOT_eType5  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(5<<2))

BERR_Code BHSM_InitKeySlot( BHSM_Handle hHsm, BHSM_InitKeySlotIO_t *pInitKeySlot )
{
    BERR_Code           errCode = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_KeyslotTypes_t ksType;
    uint8_t  status;

    BDBG_ENTER( BHSM_InitKeySlot );

    BKNI_Memset( &ksType, 0, sizeof(ksType) );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pInitKeySlot == NULL )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( hHsm->keySlotsInitialised )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED); /* Client must not try to initialise keyslots more that once. */
    }

    if( ( errCode = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( errCode );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_INIT_KEYSLOT, &header );

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    pInitKeySlot->unKeySlotType0Num += BHSM_NUM_BYPASS_KEYLSOTS; /* Bypass keyslots will be Type0 */
    #endif

    if( pInitKeySlot->numMulti2KeySlots > BCMD_MULTI2_MAXSYSKEY )
    {
        BDBG_WRN(("Maxumum supported Multi2 slots is [%d] [%d]", BCMD_MULTI2_MAXSYSKEY , pInitKeySlot->numMulti2KeySlots ));
        pInitKeySlot->numMulti2KeySlots = BCMD_MULTI2_MAXSYSKEY;
    }
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eMulti2, pInitKeySlot->numMulti2KeySlots );
    #else
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eMulti2, pInitKeySlot->numMulti2KeySlots ? 1 : 0 );
    #endif

    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType0,  pInitKeySlot->unKeySlotType0Num );
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType1,  pInitKeySlot->unKeySlotType1Num );
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType2,  pInitKeySlot->unKeySlotType2Num );
    #if BHSM_ZEUS_VERSION != BHSM_ZEUS_VERSION_CALC(3,0)
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType3,  pInitKeySlot->unKeySlotType3Num );
    #endif
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType4,  pInitKeySlot->unKeySlotType4Num );
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType5,  pInitKeySlot->unKeySlotType5Num );
    #endif

    if( ( errCode = BHSM_BspMsg_SubmitCommand( hMsg ) != BERR_SUCCESS ) )
    {
        BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    /* grab the status byte! */
    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pInitKeySlot->unStatus = (uint32_t)status;

    switch( status )
    {
        case 0: break; /* all OK! */
       #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
        case 0x20:
        {
            BDBG_WRN(("%s Keyslots already initialised. [0x%X]", __FUNCTION__, status ));
            break;
        }
       #endif
        default:
        {
           BDBG_ERR(("%s  KeySlot initialisation failed [0x%X]", __FUNCTION__, status ));
           goto BHSM_P_DONE_LABEL;
        }
    }

    hHsm->keySlotsInitialised = true;

    if( hHsm->hsmPiRunningFullRom )
    {
        /* assume that requested keyslots were allocated.*/
        ksType.numMulti2KeySlots = pInitKeySlot->numMulti2KeySlots;
        ksType.numKeySlot[ ksType.numKeySlotTypes++] = pInitKeySlot->unKeySlotType0Num;
        ksType.numKeySlot[ ksType.numKeySlotTypes++] = pInitKeySlot->unKeySlotType1Num;
        ksType.numKeySlot[ ksType.numKeySlotTypes++] = pInitKeySlot->unKeySlotType2Num;
        ksType.numKeySlot[ ksType.numKeySlotTypes++] = pInitKeySlot->unKeySlotType3Num;
        ksType.numKeySlot[ ksType.numKeySlotTypes++] = pInitKeySlot->unKeySlotType4Num;
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        ksType.numKeySlot[ ksType.numKeySlotTypes++] = pInitKeySlot->unKeySlotType5Num;
        #endif
    }
    else
    {
        /* Query output to determine how many of each slot were actually allocated. */
        BHSM_BspMsg_Get8( hMsg, BCMD_InitKeySlot_OutCmd_eMulti2SystemKeyConfig, (uint8_t*)&(ksType.numMulti2KeySlots) );

        BHSM_BspMsg_Get8( hMsg, BCMD_InitKeySlot_OutCmd_eSlotNumber+(0<<2), &(ksType.numKeySlot[ksType.numKeySlotTypes++]) );
        BHSM_BspMsg_Get8( hMsg, BCMD_InitKeySlot_OutCmd_eSlotNumber+(1<<2), &(ksType.numKeySlot[ksType.numKeySlotTypes++]) );
        BHSM_BspMsg_Get8( hMsg, BCMD_InitKeySlot_OutCmd_eSlotNumber+(2<<2), &(ksType.numKeySlot[ksType.numKeySlotTypes++]) );
        BHSM_BspMsg_Get8( hMsg, BCMD_InitKeySlot_OutCmd_eSlotNumber+(3<<2), &(ksType.numKeySlot[ksType.numKeySlotTypes++]) );
        BHSM_BspMsg_Get8( hMsg, BCMD_InitKeySlot_OutCmd_eSlotNumber+(4<<2), &(ksType.numKeySlot[ksType.numKeySlotTypes++]) );
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        BHSM_BspMsg_Get8( hMsg, BCMD_InitKeySlot_OutCmd_eSlotNumber+(5<<2), &(ksType.numKeySlot[ksType.numKeySlotTypes++]) );
        #endif

        /* check that we were allocated the slots we requested.   */
        if( pInitKeySlot->numMulti2KeySlots   != ksType.numMulti2KeySlots
            ||  pInitKeySlot->unKeySlotType0Num  != ksType.numKeySlot[0]
            ||  pInitKeySlot->unKeySlotType1Num  != ksType.numKeySlot[1]
            ||  pInitKeySlot->unKeySlotType2Num  != ksType.numKeySlot[2]
            #if BHSM_ZEUS_VERSION != BHSM_ZEUS_VERSION_CALC(3,0)
            ||  pInitKeySlot->unKeySlotType3Num  != ksType.numKeySlot[3]
            #endif
            ||  pInitKeySlot->unKeySlotType4Num  != ksType.numKeySlot[4]
            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
            ||  pInitKeySlot->unKeySlotType5Num  != ksType.numKeySlot[5]
            #endif
            )
        {
            BDBG_WRN(("Some requested slots not allocated."));
        }
    }

    /* Finished with the BSP MSG object */
    (void)BHSM_BspMsg_Destroy( hMsg );
    hMsg = NULL;

    if( ( errCode = BHSM_P_KeySlotsInitialise( hHsm, &ksType ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    if( ( errCode = AllocateByPassKeyslots( hHsm )  ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    if( ( errCode = BHSM_InitialiseBypassKeyslots( hHsm ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    if( ( errCode = InitialisePidChannels( hHsm )  ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }
    #endif

    if( ( errCode = BHSM_InitialiseKeyLadders( hHsm ) ) != BERR_SUCCESS )
    {
        /* Failed to load keyladder configuration. */
        BERR_TRACE(errCode);
        goto BHSM_P_DONE_LABEL;
    }

    #if( BHSM_HOST_SAGE_SRAM_INTERFACE )
    BHSM_P_StashKeySlotTableWrite( hHsm, &ksType );
    #endif

BHSM_P_DONE_LABEL:

    if( hMsg ) {
        (void)BHSM_BspMsg_Destroy( hMsg );
        hMsg = NULL;
    }

    BDBG_LEAVE( BHSM_InitKeySlot );
    return errCode;
}


BERR_Code BHSM_LocateCAKeySlotAssigned (
        BHSM_Handle                    hHsm,
        unsigned int                in_unPidChannel,
        BHSM_PidChannelType_e        in_pidChannelType,
        BCMD_XptSecKeySlot_e        *outp_ucKeySlotType,
        unsigned int                *pKeySlotNumber
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BHSM_LocateCAKeySlotAssigned);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if(  outp_ucKeySlotType == NULL  || pKeySlotNumber == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    BDBG_MSG(("BHSM_HasKeySlotAssigned in_unPidChannel=%d, in_pidChannelType = %d", in_unPidChannel, in_pidChannelType));
    BDBG_MSG(("keySlotType = %d, unKeySlotNum = %d",
    hHsm->mapPidChannelToKeySlot[in_unPidChannel][in_pidChannelType].keySlotType ,
    hHsm->mapPidChannelToKeySlot[in_unPidChannel][in_pidChannelType].unKeySlotNum));


    if ( (hHsm->mapPidChannelToKeySlot[in_unPidChannel][in_pidChannelType].keySlotType
            != BCMD_XptSecKeySlot_eTypeMax ) &&
        (hHsm->mapPidChannelToKeySlot[in_unPidChannel][in_pidChannelType].unKeySlotNum
            !=  BHSM_SLOT_NUM_INIT_VAL) )
    {
        *outp_ucKeySlotType = hHsm->mapPidChannelToKeySlot[in_unPidChannel][in_pidChannelType].keySlotType ;
        *pKeySlotNumber = hHsm->mapPidChannelToKeySlot[in_unPidChannel][in_pidChannelType].unKeySlotNum ;
    }
    else
    {
        *outp_ucKeySlotType = BCMD_XptSecKeySlot_eTypeMax ;
        *pKeySlotNumber = BHSM_SLOT_NUM_INIT_VAL;
    }

    BDBG_LEAVE(BHSM_LocateCAKeySlotAssigned);
    return( errCode );
}

static BERR_Code ConfigPidKeyPointerTable (
        BHSM_Handle                           hHsm,
        BHSM_ConfigPidKeyPointerTableIO_t    *pPidChannelConf,
        bool verbose )
{
    BERR_Code           errCode = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint32_t            tmp = 0;
    uint8_t             status = 0;

    BDBG_ENTER( ConfigPidKeyPointerTable );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pPidChannelConf == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if(   pPidChannelConf->ucKeySlotType >= BCMD_XptSecKeySlot_eTypeMax ||    /* invalid slot type */
        ( pPidChannelConf->unKeySlotNum >= hHsm->keySlotTypes[pPidChannelConf->ucKeySlotType].unKeySlotNum ) )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( pPidChannelConf->unKeySlotBType >= BCMD_XptSecKeySlot_eTypeMax ||     /* invalid slot type */
        ( hHsm->keySlotTypes[pPidChannelConf->unKeySlotBType].unKeySlotNum  &&
          (pPidChannelConf->unKeySlotNumberB >= hHsm->keySlotTypes[pPidChannelConf->unKeySlotBType].unKeySlotNum)) )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( errCode = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    header.verbose = verbose;
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_CONFIG_PIDKEYPOINTERTABLE, &header );

    tmp  = pPidChannelConf->unPidChannel;
    tmp |= ( (pPidChannelConf->bResetPIDToDefault?1:0) << 30 );
    tmp |= (  pPidChannelConf->spidProgType << 31 );

    BHSM_BspMsg_Pack32( hMsg, BCMD_KeyPointer_InCmdCfg_ePidChan, tmp );

    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotType, pPidChannelConf->ucKeySlotType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotNumber, pPidChannelConf->unKeySlotNum );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotTypeB, pPidChannelConf->unKeySlotBType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotNumberB, pPidChannelConf->unKeySlotNumberB );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eKeyPointerSel, pPidChannelConf->unKeyPointerSel );

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    if (hHsm->firmwareVersion.bseck.major >= BHSM_BSP_FW_VERSION_KEYSLOT_MULTIPLE_PID_CHANNELS )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSetMultiplePidChan, pPidChannelConf->setMultiplePidChannels ? 1 : 0);

        if (pPidChannelConf->setMultiplePidChannels)
        {
            if(pPidChannelConf->unPidChannelEnd >= BCMD_TOTAL_PIDCHANNELS)
            {
                BERR_TRACE( BERR_INVALID_PARAMETER );
                goto BHSM_P_DONE_LABEL;
            }

            BHSM_BspMsg_Pack16( hMsg, BCMD_KeyPointer_InCmdCfg_ePidChanEnd, pPidChannelConf->unPidChannelEnd );
        }
    }
   #endif

    errCode = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( errCode != BERR_SUCCESS )
    {
        BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pPidChannelConf->unStatus = status;
    if( status != 0 )
    {
        errCode = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP Status error [0x%X]", __FUNCTION__, status ));
        goto BHSM_P_DONE_LABEL;
    }

    /* Keep track of which key slot associate with which pid channel */
    hHsm->mapPidChannelToKeySlot[pPidChannelConf->unPidChannel][pPidChannelConf->pidChannelType].keySlotType =
                pPidChannelConf->ucKeySlotType;
    hHsm->mapPidChannelToKeySlot[pPidChannelConf->unPidChannel][pPidChannelConf->pidChannelType].unKeySlotNum =
                pPidChannelConf->unKeySlotNum;

BHSM_P_DONE_LABEL:
    if( hMsg )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( ConfigPidKeyPointerTable );
    return errCode;
}


BERR_Code BHSM_ConfigPidKeyPointerTable (
        BHSM_Handle                           hHsm,
        BHSM_ConfigPidKeyPointerTableIO_t    *pPidChannelConf
)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_ConfigPidKeyPointerTable );

    BDBG_MSG(("%s Type[%d] keySlot[%d]", __FUNCTION__, pPidChannelConf->ucKeySlotType, pPidChannelConf->unKeySlotNum ));

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    pPidChannelConf->setMultiplePidChannels = false;
   #endif

    if( ( rc = ConfigPidKeyPointerTable ( hHsm, pPidChannelConf, true/*verbose, if debug is enabled*/ ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BDBG_LEAVE( BHSM_ConfigPidKeyPointerTable );
    return BERR_SUCCESS;
}

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)

BERR_Code BHSM_AllocateCAKeySlot ( /* Zeus 4 */
    BHSM_Handle                  hHsm,
    BHSM_XptKeySlotIO_t         *pKeySlotConf
)
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned int i = 0;
    unsigned int keySlotOffset = 0;
    unsigned int maxKeySlot = 0;
    unsigned     keySlotNum;
    BCMD_XptSecKeySlot_e keySlotType;
    BHSM_InvalidateKeyIO_t  resetKeySlot;
    BHSM_P_CAKeySlotInfo_t  *pKeyslot = NULL;
    BHSM_KeySlotOwnership_t ownership;

    BDBG_ENTER(BHSM_AllocateCAKeySlot);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pKeySlotConf == NULL )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    keySlotType = pKeySlotConf->keySlotType;
    pKeySlotConf->keySlotNum = BHSM_SLOT_NUM_INIT_VAL; /* default return value */

    if( keySlotType >= BCMD_XptSecKeySlot_eTypeMax )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    /* Search for vacant key slot */
    keySlotOffset = hHsm->keySlotTypes[keySlotType].ucKeySlotStartOffset;
    maxKeySlot    = hHsm->keySlotTypes[keySlotType].unKeySlotNum;

    /* If SAGE client, check for an available SAGE slot. */
    if( pKeySlotConf->client == BHSM_ClientType_eSAGE )
    {
        i = keySlotOffset;
        while( i < (keySlotOffset + maxKeySlot) )
        {
            if( hHsm->pKeySlot[i] == NULL )
            {
                /* inconsistent data */
                errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            /* search first free SAGE slot. */
            if( ( hHsm->pKeySlot[i]->bIsUsed == false ) && ( hHsm->pKeySlot[i]->client == BHSM_ClientType_eSAGE ) )
            {
                pKeyslot = hHsm->pKeySlot[i];   /* Found */
                break;
            }
            i++;
        }
    }

    if( pKeyslot == NULL )
    {
        /* search again without caring about slot's client type. */
        i = keySlotOffset;
        while( i < (keySlotOffset + maxKeySlot ))
        {
            if( hHsm->pKeySlot[i] == NULL )
            {
                /* inconsistent data */
                errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            /* Search for first FREE or SHARED key slot */
            if( ( hHsm->pKeySlot[i]->bIsUsed == false ) && ( hHsm->pKeySlot[i]->client != BHSM_ClientType_eSAGE ) )
            {
                pKeyslot = hHsm->pKeySlot[i]; /* Found */
                break;
            }
            i++;
        }
    }

    if( pKeyslot == NULL )  /* we STILL did not find a slot. */
    {
        BDBG_ERR(("%s Failed to find a free slot. type[%d] offset[%d] max[%d]", __FUNCTION__, keySlotType, keySlotOffset, i ));
        errCode = BERR_TRACE( BHSM_STATUS_FAILED );
        goto BHSM_P_DONE_LABEL;
    }

    /* Here is the vacant key slot */
    keySlotNum = i - keySlotOffset;

    if( (pKeyslot->client == BHSM_ClientType_eHost) || (pKeyslot->client == BHSM_ClientType_eNone) ) /* we can't invalidate a SAGE keyslot. */
    {
        /* Invalidate the keyslot. */
        BKNI_Memset( &resetKeySlot, 0, sizeof(resetKeySlot) );
        resetKeySlot.caKeySlotType      = keySlotType;
        resetKeySlot.unKeySlotNum       = keySlotNum;
        resetKeySlot.keyDestBlckType    = BCMD_KeyDestBlockType_eCPScrambler;
        resetKeySlot.invalidKeyType     = BCMD_InvalidateKey_Flag_eDestKeyOnly;
        resetKeySlot.keyDestEntryType   = BCMD_KeyDestEntryType_eOddKey;
        resetKeySlot.virtualKeyLadderID = BCMD_VKL_KeyRam_eMax; /*Not used. Setting to invalid value to avoid collision with VKL in use*/
        resetKeySlot.bInvalidateAllEntries     = true;
        errCode = BHSM_InvalidateKey( hHsm, &resetKeySlot );
        if( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("%s BHSM_InvalidateKey failed type[%d] num[%d] continuing", __FUNCTION__, keySlotType, keySlotNum ));
            goto BHSM_P_DONE_LABEL;
        }
    }

    BKNI_Memset( &ownership, 0, sizeof(ownership) );
    switch( pKeySlotConf->client )
    {
        case BHSM_ClientType_eSAGE:  { ownership.owner = BHSM_KeySlotOwner_eSAGE;    break; }
        case BHSM_ClientType_eHost:  { ownership.owner = BHSM_KeySlotOwner_eSHARED;  break; }
        default:                     { errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); goto BHSM_P_DONE_LABEL; }
    }
    ownership.keySlotNumber = keySlotNum;
    ownership.keySlotType   = keySlotType;

    if( ( errCode = BHSM_SetKeySlotOwnership( hHsm, &ownership ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    pKeySlotConf->keySlotNum = keySlotNum;

    BKNI_Memset( pKeyslot, 0, sizeof(BHSM_P_CAKeySlotInfo_t) );
    pKeyslot->bIsUsed = true;
    pKeyslot->client  = pKeySlotConf->client;
    pKeyslot->keySlotType = pKeySlotConf->keySlotType;
    pKeyslot->keySlotNum = pKeySlotConf->keySlotNum;
    pKeyslot->ulGlobalControlWordHi = 0;
    pKeyslot->ulGlobalControlWordLo = BHSM_ALL_GLOBAL_REGION_MAP;

    BDBG_MSG(("KeySlot Allocated - Type[%d] Number[%d] client[%s]", keySlotType, keySlotNum,  pKeySlotConf->client == BHSM_ClientType_eSAGE ? "Sage":"Host" ));

BHSM_P_DONE_LABEL:

    pKeySlotConf->unStatus = errCode;

    BDBG_LEAVE( BHSM_AllocateCAKeySlot );
    return errCode;
}

BERR_Code BHSM_FreeCAKeySlot ( /* Zeus 4 */
            BHSM_Handle hHsm,
            BHSM_XptKeySlotIO_t *pKeySlotConf )
{
    BHSM_P_CAKeySlotInfo_t *pKeyslot;
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    i = 0;
    unsigned    extKsNum = 0;
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    BHSM_InvalidateKeyIO_t  resetKeySlot;
    BHSM_KeySlotOwnership_t ownership;
    #endif

    BDBG_ENTER( BHSM_FreeCAKeySlot );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pKeySlotConf == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( ( errCode =  BHSM_P_GetKeySlot( hHsm,
                                       &pKeyslot,
                                        pKeySlotConf->keySlotType,
                                        pKeySlotConf->keySlotNum ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( errCode );
    }

    /* Return if the key slot is already empty */
    if( pKeyslot->bIsUsed == false )
    {
        BDBG_ERR(("%s  slot is already free type[%d] slotNum[%d]", __FUNCTION__, pKeySlotConf->keySlotType,  pKeySlotConf->keySlotNum ));
        goto BHSM_P_DONE_LABEL;
    }

     /* flag the keyslot as allocated at BSP */
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)

    if( pKeyslot->client == BHSM_ClientType_eHost )
    {
        /* Invalidate the keyslot. */
        BKNI_Memset( &resetKeySlot, 0, sizeof(resetKeySlot) );
        resetKeySlot.caKeySlotType      = pKeySlotConf->keySlotType;
        resetKeySlot.keyDestBlckType    = BCMD_KeyDestBlockType_eCPScrambler;
        resetKeySlot.unKeySlotNum       = pKeySlotConf->keySlotNum;
        resetKeySlot.invalidKeyType     = BCMD_InvalidateKey_Flag_eDestKeyOnly;
        resetKeySlot.keyDestEntryType   = BCMD_KeyDestEntryType_eOddKey;
        resetKeySlot.virtualKeyLadderID = BCMD_VKL_KeyRam_eMax; /*Not used. Setting to invalid value to avoid collision with VKL in use*/
        resetKeySlot.bInvalidateAllEntries = true;
        errCode = BHSM_InvalidateKey( hHsm, &resetKeySlot );
        if( errCode != BERR_SUCCESS )
        {
           BDBG_ERR(( "%s BHSM_InvalidateKey failed type[%d] num[%d] continuing", __FUNCTION__, pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum ));
           goto BHSM_P_DONE_LABEL;
        }
    }

    /*Poll BSP for keyslot ownership, it may have been updated, i.e., freed from SAGE*/
    BKNI_Memset( &ownership, 0, sizeof(ownership) );
    ownership.keySlotNumber = pKeySlotConf->keySlotNum;
    ownership.keySlotType   = pKeySlotConf->keySlotType;
    if( ( errCode = BHSM_GetKeySlotOwnership( hHsm, &ownership ) ) !=  BERR_SUCCESS )
    {
       (void)BERR_TRACE( errCode );
       goto BHSM_P_DONE_LABEL;
    }

    /* reevaluate the client associated with the keyslot. */
    switch( ownership.owner )
    {
        case BHSM_KeySlotOwner_eFREE:
        {
            /* this is what's expected.  */
            pKeyslot->client = BHSM_ClientType_eNone;
            break;
        }
        case BHSM_KeySlotOwner_eSAGE:
        {
            BDBG_WRN(("%s SAGE did not invalidate Keyslot type[%d] num[%d]", __FUNCTION__, pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum ));

            /*We do expect the keyslot client to be SAGE */
            if( pKeyslot->client != BHSM_ClientType_eSAGE )
            {
                BDBG_WRN(( "%s Keyslot has inconsistant client type[%d] num[%d] client[%d]", __FUNCTION__, pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum, pKeyslot->client ));
            }
            pKeyslot->client = BHSM_ClientType_eSAGE;
            break;
        }
        case BHSM_KeySlotOwner_eSHARED:
        {
            BDBG_WRN(( "%s HOST Keyslot invalidate failed. type[%d] num[%d]", __FUNCTION__, pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum ));
            pKeyslot->client = BHSM_ClientType_eHost;
            break;
        }
        default:
        {
            /* Invalid ownership value. */
            errCode = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
            goto BHSM_P_DONE_LABEL;
        }
    }

   #endif

    /* free external key slots */
    for( i = 0; i < BCMD_KeyDestEntryType_eMax; i++ )
    {
        if( pKeyslot->aKeySlotAlgorithm[i].configured )
        {
            if( pKeyslot->aKeySlotAlgorithm[i].externalKeySlot.valid )
            {
                extKsNum = pKeyslot->aKeySlotAlgorithm[i].externalKeySlot.slotNum;
                if( hHsm->externalKeySlotTable[extKsNum].allocated  == false )
                {
                    errCode = BERR_TRACE( BHSM_STATUS_FAILED ); /*continue, best effort*/
                }
                hHsm->externalKeySlotTable[extKsNum].allocated = false;
                hHsm->externalKeySlotTable[extKsNum].key.valid = false;
                hHsm->externalKeySlotTable[extKsNum].iv.valid = false;
                pKeyslot->aKeySlotAlgorithm[i].externalKeySlot.valid = false;
            }
        }
    }

    pKeyslot->bIsUsed = false;

    BDBG_MSG(("KeySlot Freed - Type[%d] Number[%d]", pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum ));

    BHSM_P_DONE_LABEL:

    pKeySlotConf->unStatus = errCode;

    BDBG_LEAVE(BHSM_FreeCAKeySlot);
    return errCode;
}


BERR_Code BHSM_AllocateM2MKeySlot (   /*  Zeus 4 */
        BHSM_Handle                    hHsm,
        BHSM_M2MKeySlotIO_t         *pM2mKeySlot
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BHSM_XptKeySlotIO_t XptKeySlotIO;

    BDBG_ENTER(BHSM_AllocateM2MKeySlot);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pM2mKeySlot == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( ( pM2mKeySlot->keySlotType != BCMD_XptSecKeySlot_eType0 ) &&
        ( pM2mKeySlot->keySlotType != BCMD_XptSecKeySlot_eType2 ) &&
        ( pM2mKeySlot->keySlotType != BCMD_XptSecKeySlot_eType3 ) &&
        ( pM2mKeySlot->keySlotType != BCMD_XptSecKeySlot_eType5 ) )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    BKNI_Memset( &XptKeySlotIO, 0 , sizeof(XptKeySlotIO) );

    XptKeySlotIO.client = pM2mKeySlot->client;
    XptKeySlotIO.keySlotType = pM2mKeySlot->keySlotType;

    /*  Call BHSM_AllocateCAKeySlot with the requested key slot type */
    errCode = BHSM_AllocateCAKeySlot( hHsm, &XptKeySlotIO );

    BDBG_MSG(("%s  Client[%d] Type[%d] Slot[%d]" ,  __FUNCTION__,  pM2mKeySlot->client,  pM2mKeySlot->keySlotType, XptKeySlotIO.keySlotNum ));

    pM2mKeySlot->unStatus   = errCode;
    pM2mKeySlot->keySlotNum = XptKeySlotIO.keySlotNum;

    BDBG_LEAVE(BHSM_AllocateM2MKeySlot);
    return errCode;
}

BERR_Code BHSM_FreeM2MKeySlot ( /* Zeus 4 */
    BHSM_Handle                    hHsm,
    BHSM_M2MKeySlotIO_t       *pM2mKeySlot
)
{
    BERR_Code      errCode = BERR_SUCCESS;
    BHSM_XptKeySlotIO_t XptKeySlotIO;

    BDBG_ENTER(BHSM_FreeM2MKeySlot);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pM2mKeySlot == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( ( pM2mKeySlot->keySlotType != BCMD_XptSecKeySlot_eType0 ) &&
        ( pM2mKeySlot->keySlotType != BCMD_XptSecKeySlot_eType2 ) &&
        ( pM2mKeySlot->keySlotType != BCMD_XptSecKeySlot_eType3 ) &&
        ( pM2mKeySlot->keySlotType != BCMD_XptSecKeySlot_eType5 ) )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); /* invalid slot type. */
    }

    if( pM2mKeySlot->keySlotNum >= (hHsm->keySlotTypes[pM2mKeySlot->keySlotType].unKeySlotNum) )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); /* invalid slot number. */
    }

    BKNI_Memset( &XptKeySlotIO,0 , sizeof(XptKeySlotIO) );

    XptKeySlotIO.client         = pM2mKeySlot->client;
    XptKeySlotIO.keySlotNum     = pM2mKeySlot->keySlotNum;
    XptKeySlotIO.keySlotType    = pM2mKeySlot->keySlotType;
    XptKeySlotIO.pidChannelType = BHSM_PidChannelType_ePrimary;

    errCode = BHSM_FreeCAKeySlot(hHsm, &XptKeySlotIO);

    BDBG_MSG(("%s Client[%d] Type[%d] Slot[%d]",  __FUNCTION__, pM2mKeySlot->client, pM2mKeySlot->keySlotType, pM2mKeySlot->keySlotNum ));

    BDBG_LEAVE(BHSM_FreeM2MKeySlot);
    return errCode;
}

#else   /* pre Zeus 4.0  ... 40 nm parts */

BERR_Code BHSM_AllocateCAKeySlot_v2 ( /* pre Zeus 4 */
        BHSM_Handle             hHsm,
        BHSM_KeySlotAllocate_t *pKeySlotConf
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned int i = 0;
    unsigned int keySlotOffset = 0;
    unsigned int maxKeySlot = 0;
    unsigned     keySlotNum;
    BCMD_XptSecKeySlot_e keySlotType;
    BHSM_InvalidateKeyIO_t  resetKeySlot;
    BHSM_P_CAKeySlotInfo_t  *pKeyslot = NULL;

    BDBG_ENTER(BHSM_AllocateCAKeySlot);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pKeySlotConf == NULL )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    keySlotType = pKeySlotConf->keySlotType;
    pKeySlotConf->keySlotNum = BHSM_SLOT_NUM_INIT_VAL; /* default return value */

    if( keySlotType >= BCMD_XptSecKeySlot_eTypeMax )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    /* Search for vacant key slot */
    keySlotOffset = hHsm->keySlotTypes[keySlotType].ucKeySlotStartOffset;
    maxKeySlot    = hHsm->keySlotTypes[keySlotType].unKeySlotNum;

    /* If SAGE client, check for an available SAGE slot. */
    if( isKeySlotOwershipSupported( hHsm ) )
    {
        if( pKeySlotConf->client == BHSM_ClientType_eSAGE )
        {
            i = keySlotOffset;
            while( i < (keySlotOffset + maxKeySlot) )
            {
                if( hHsm->pKeySlot[i] == NULL )
                {
                    /* inconsistent data */
                    errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );
                    goto BHSM_P_DONE_LABEL;
                }

                /* search first free SAGE slot. */
                if( ( hHsm->pKeySlot[i]->bIsUsed == false ) && ( hHsm->pKeySlot[i]->client == BHSM_ClientType_eSAGE ) )
                {
                    pKeyslot = hHsm->pKeySlot[i];   /* Found */
                    break;
                }
                i++;
            }
        }
    }

    if( pKeyslot == NULL )
    {
        /* search again. */
        i = keySlotOffset;
        while( i < (keySlotOffset + maxKeySlot ))
        {
            if( hHsm->pKeySlot[i] == NULL )
            {
                /* inconsistent data */
                errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            /* Search for first FREE or SHARED key slot */
            if( ( hHsm->pKeySlot[i]->bIsUsed == false ) && ( hHsm->pKeySlot[i]->client != BHSM_ClientType_eSAGE ) )
            {
                pKeyslot = hHsm->pKeySlot[i]; /* Found */
                break;
            }
            i++;
        }
    }

    if( pKeyslot == NULL )  /* we STILL did not find a slot. */
    {
        BDBG_ERR(("%s Failed to find a free slot. type[%d] offset[%d] max[%d]", __FUNCTION__, keySlotType, keySlotOffset, i ));
        errCode = BERR_TRACE( BHSM_STATUS_FAILED );
        goto BHSM_P_DONE_LABEL;
    }

    /* Here is the vacant key slot */
    keySlotNum = i - keySlotOffset;

    if( pKeyslot->client != BHSM_ClientType_eSAGE ) /* we can't invalidate a SAGE keyslot. */
    {
        /* Invalidate the keyslot. */
        BKNI_Memset( &resetKeySlot, 0, sizeof(resetKeySlot) );
        resetKeySlot.caKeySlotType      = keySlotType;
        resetKeySlot.unKeySlotNum       = keySlotNum;
        resetKeySlot.keyDestBlckType    = BCMD_KeyDestBlockType_eCPScrambler;
        resetKeySlot.invalidKeyType     = BCMD_InvalidateKey_Flag_eDestKeyOnly;
        resetKeySlot.keyDestEntryType   = BCMD_KeyDestEntryType_eOddKey;
        resetKeySlot.virtualKeyLadderID = BCMD_VKL_KeyRam_eMax; /*Not used. Setting to invalid value to avoid collision with VKL in use*/
        resetKeySlot.bInvalidateAllEntries     = true;
        errCode = BHSM_InvalidateKey( hHsm, &resetKeySlot );
        if( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("%s BHSM_InvalidateKey failed type[%d] num[%d] continuing", __FUNCTION__, keySlotType, keySlotNum ));
            goto BHSM_P_DONE_LABEL;
        }
    }

    if( isKeySlotOwershipSupported( hHsm ) )
    {
        BHSM_KeySlotOwnership_t ownership;

        BKNI_Memset( &ownership, 0, sizeof(ownership) );
        switch( pKeySlotConf->client )
        {
            case BHSM_ClientType_eSAGE:  { ownership.owner = BHSM_KeySlotOwner_eSAGE;    break; }
            case BHSM_ClientType_eHost:  { ownership.owner = BHSM_KeySlotOwner_eSHARED;  break; }
            default:                     { errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); goto BHSM_P_DONE_LABEL; }
        }
        ownership.keySlotNumber = keySlotNum;
        ownership.keySlotType   = keySlotType;
        if( ( errCode = BHSM_SetKeySlotOwnership( hHsm, &ownership ) ) != BERR_SUCCESS )
        {
            BERR_TRACE( errCode );
            goto BHSM_P_DONE_LABEL;
        }
    }

    pKeySlotConf->keySlotNum = keySlotNum;

    BKNI_Memset( pKeyslot, 0, sizeof(BHSM_P_CAKeySlotInfo_t) );
    pKeyslot->bIsUsed     = true;
    pKeyslot->client      = pKeySlotConf->client;
    pKeyslot->keySlotType = pKeySlotConf->keySlotType;
    pKeyslot->keySlotNum  = pKeySlotConf->keySlotNum;
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    pKeyslot->ulGlobalControlWordHi = 0;
    pKeyslot->ulGlobalControlWordLo = BHSM_ALL_GLOBAL_REGION_MAP;
   #endif

    BDBG_MSG(("KeySlot Allocated - Type[%d] Number[%d] client[%s]", keySlotType, keySlotNum,  pKeySlotConf->client == BHSM_ClientType_eSAGE ? "Sage":"Host" ));

BHSM_P_DONE_LABEL:

    pKeySlotConf->unStatus = errCode;

    BDBG_LEAVE( BHSM_AllocateCAKeySlot );
    return errCode;
}


BERR_Code BHSM_FreeCAKeySlot_v2 ( /* pre Zeus 4 */
            BHSM_Handle hHsm,
            BHSM_KeySlotAllocate_t *pKeySlotConf )
{
    BHSM_P_CAKeySlotInfo_t *pKeyslot;
    BERR_Code   errCode = BERR_SUCCESS;
    BHSM_InvalidateKeyIO_t  resetKeySlot;

    BDBG_ENTER( BHSM_FreeCAKeySlot );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pKeySlotConf == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( ( errCode =  BHSM_P_GetKeySlot( hHsm,
                                       &pKeyslot,
                                        pKeySlotConf->keySlotType,
                                        pKeySlotConf->keySlotNum ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( errCode );
    }

    /* Return if the key slot is already empty */
    if( pKeyslot->bIsUsed == false )
    {
        BDBG_ERR(("%s  slot is already free type[%d] slotNum[%d]", __FUNCTION__, pKeySlotConf->keySlotType,  pKeySlotConf->keySlotNum ));
        goto BHSM_P_DONE_LABEL;
    }

    /* flag the keyslot as allocated at BSP */
    if( pKeyslot->client == BHSM_ClientType_eHost )
    {
        /* Invalidate the keyslot. */
        BKNI_Memset( &resetKeySlot, 0, sizeof(resetKeySlot) );
        resetKeySlot.caKeySlotType      = pKeySlotConf->keySlotType;
        resetKeySlot.keyDestBlckType    = BCMD_KeyDestBlockType_eCPScrambler;
        resetKeySlot.unKeySlotNum       = pKeySlotConf->keySlotNum;
        resetKeySlot.invalidKeyType     = BCMD_InvalidateKey_Flag_eDestKeyOnly;
        resetKeySlot.keyDestEntryType   = BCMD_KeyDestEntryType_eOddKey;
        resetKeySlot.virtualKeyLadderID = BCMD_VKL_KeyRam_eMax; /*Not used. Setting to invalid value to avoid collision with VKL in use*/
        resetKeySlot.bInvalidateAllEntries = true;
        errCode = BHSM_InvalidateKey( hHsm, &resetKeySlot );
        if( errCode != BERR_SUCCESS )
        {
           BDBG_ERR(( "%s BHSM_InvalidateKey failed type[%d] num[%d] continuing", __FUNCTION__, pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum ));
           goto BHSM_P_DONE_LABEL;
        }
    }

    if( isKeySlotOwershipSupported( hHsm ) )
    {
        BHSM_KeySlotOwnership_t ownership;

        BKNI_Memset( &ownership, 0, sizeof(ownership) );
        ownership.keySlotNumber = pKeySlotConf->keySlotNum;
        ownership.keySlotType   = pKeySlotConf->keySlotType;

        /*Poll BSP for keyslot ownership, it may have been updated, i.e., freed from SAGE*/
        if( ( errCode = BHSM_GetKeySlotOwnership( hHsm, &ownership ) ) !=  BERR_SUCCESS )
        {
           (void)BERR_TRACE( errCode );
           goto BHSM_P_DONE_LABEL;
        }

        /* reevaluate the client associated with the keyslot. */
        switch( ownership.owner )
        {
            case BHSM_KeySlotOwner_eFREE:
            {
                /* this is what's expected.  */
                pKeyslot->client = BHSM_ClientType_eNone;
                break;
            }
            case BHSM_KeySlotOwner_eSAGE:
            {
                BDBG_WRN(("%s SAGE did not invalidate Keyslot type[%d] num[%d]", __FUNCTION__, pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum ));

                /*We do expect the keyslot client to be SAGE */
                if( pKeyslot->client != BHSM_ClientType_eSAGE )
                {
                    BDBG_WRN(( "%s Keyslot has inconsistant client type[%d] num[%d] client", __FUNCTION__, pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum, pKeyslot->client ));
                }
                pKeyslot->client = BHSM_ClientType_eSAGE;
                break;
            }
            case BHSM_KeySlotOwner_eSHARED:
            {
                BDBG_WRN(( "%s HOST Keyslot invalidate failed. type[%d] num[%d]", __FUNCTION__, pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum ));
                pKeyslot->client = BHSM_ClientType_eHost;
                break;
            }
            default:
            {
                /* Invalid ownership value. */
                errCode = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
                goto BHSM_P_DONE_LABEL;
            }
        }
    }

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
   /* free external key slots */
    {
        unsigned    i = 0;
        unsigned    extKsNum = 0;
        for( i = 0; i < BCMD_KeyDestEntryType_eMax; i++ )
        {
            if( pKeyslot->aKeySlotAlgorithm[i].configured )
            {
                if( pKeyslot->aKeySlotAlgorithm[i].externalKeySlot.valid )
                {
                    extKsNum = pKeyslot->aKeySlotAlgorithm[i].externalKeySlot.slotNum;
                    if( hHsm->externalKeySlotTable[extKsNum].allocated  == false )
                    {
                        errCode = BERR_TRACE( BHSM_STATUS_FAILED ); /*continue, best effort*/
                    }
                    hHsm->externalKeySlotTable[extKsNum].allocated = false;
                    hHsm->externalKeySlotTable[extKsNum].key.valid = false;
                    hHsm->externalKeySlotTable[extKsNum].iv.valid = false;
                    pKeyslot->aKeySlotAlgorithm[i].externalKeySlot.valid = false;
                }
            }
        }
    }
    #endif

    pKeyslot->bIsUsed = false;

    BDBG_MSG(("KeySlot Freed - Type[%d] Number[%d]", pKeySlotConf->keySlotType, pKeySlotConf->keySlotNum ));

    BHSM_P_DONE_LABEL:

    pKeySlotConf->unStatus = errCode;

    BDBG_LEAVE(BHSM_FreeCAKeySlot);
    return errCode;
}


BERR_Code BHSM_AllocateCAKeySlot (  /* Pre Zeus 4 */
    BHSM_Handle             hHsm,
    BCMD_XptSecKeySlot_e    keySlotType,
    unsigned                *pKeySlotNumber )
{
    BHSM_KeySlotAllocate_t slotConf;
    BERR_Code rc;

    BKNI_Memset( &slotConf, 0, sizeof(slotConf) );
    slotConf.client         = BHSM_ClientType_eHost; /* default */
    slotConf.keySlotType    = keySlotType;
    slotConf.pidChannel     = DUMMY_PID_CHANNEL_TOAKEYSLOT;
    slotConf.pidChannelType = BHSM_PidChannelType_ePrimary;

    if( ( rc = BHSM_AllocateCAKeySlot_v2( hHsm, &slotConf ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }
    *pKeySlotNumber = slotConf.keySlotNum;

    return rc;
}

BERR_Code BHSM_FreeCAKeySlot (   /* Pre Zeus 4 */
    BHSM_Handle            hHsm,
    unsigned               inPidChannel,
    BHSM_PidChannelType_e  inPidChannelType,
    BCMD_XptSecKeySlot_e   keySlotType,
    unsigned int           inKeySlotNum  )
{
    BERR_Code rc;
    BHSM_KeySlotAllocate_t slotConf;

    BKNI_Memset( &slotConf, 0, sizeof(slotConf) );
    slotConf.client         = BHSM_ClientType_eHost; /* default */
    slotConf.keySlotType    = keySlotType;
    slotConf.pidChannel     = inPidChannel;
    slotConf.pidChannelType = inPidChannelType;
    slotConf.keySlotNum     = inKeySlotNum;

    if( ( rc = BHSM_FreeCAKeySlot_v2( hHsm, &slotConf ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    return rc;
}

BERR_Code BHSM_AllocateM2MKeySlot (  /* Pre Zeus 4 */
    BHSM_Handle                  hHsm,
    BHSM_M2MKeySlotIO_t         *pM2mKeySlot
)
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned  slotNumber;
    BHSM_P_M2MKeySlotInfo_t *pKeyslot = NULL;

    BDBG_ENTER( BHSM_AllocateM2MKeySlot );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pM2mKeySlot == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    pM2mKeySlot->keySlotNum = BHSM_SLOT_NUM_INIT_VAL;

    /* search  without caring about slot's client type. */
    for( slotNumber = 0; slotNumber < BCMD_MAX_M2M_KEY_SLOT; slotNumber++ )
    {
        if( hHsm->aunM2MKeySlotInfo[slotNumber].bIsUsed == false )
        {
            pKeyslot = &hHsm->aunM2MKeySlotInfo[slotNumber];
            break;
        }
    }

    if( pKeyslot == NULL )  /* we STILL did not find a slot. */
    {
        errCode = BERR_TRACE( BHSM_STATUS_FAILED );  /* out of keyslots. */
        goto BHSM_P_DONE_LABEL;
    }

    /* configure the keyslot record */
    BKNI_Memset( pKeyslot,  0x00, sizeof(*pKeyslot) );
    pKeyslot->bIsUsed = true;
    pKeyslot->client = pM2mKeySlot->client;

    /* Send key slot number back to caller  */
    pM2mKeySlot->keySlotNum = slotNumber;
    pM2mKeySlot->unStatus   = errCode;

    BDBG_MSG(("BHSM_AllocateM2MKeySlot allocated M2M keyslot [#%d]", pM2mKeySlot->keySlotNum));

BHSM_P_DONE_LABEL:

    BDBG_LEAVE( BHSM_AllocateM2MKeySlot );
    return errCode;
}

BERR_Code BHSM_FreeM2MKeySlot ( /* Pre Zeus 4 */
    BHSM_Handle            hHsm,
    BHSM_M2MKeySlotIO_t    *pM2mKeySlot
)
{
    BERR_Code    errCode = BERR_SUCCESS;
    BHSM_P_M2MKeySlotInfo_t *pKeyslot = NULL;
    BHSM_InvalidateKeyIO_t invalidate;

    BDBG_ENTER( BHSM_FreeM2MKeySlot );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pM2mKeySlot == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( pM2mKeySlot->keySlotNum >= BCMD_MAX_M2M_KEY_SLOT )
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    BDBG_MSG(("%s  key slot = %d", __FUNCTION__,  pM2mKeySlot->keySlotNum ));

    pKeyslot = &hHsm->aunM2MKeySlotInfo[pM2mKeySlot->keySlotNum];

    /* Return if the key slot is already empty */
    if( pKeyslot->bIsUsed == false )
    {
        BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR); /* continue for best effort */
    }

    /* invalidate keyslot */
    BKNI_Memset( &invalidate, 0, sizeof(invalidate) );
    invalidate.invalidKeyType        = BCMD_InvalidateKey_Flag_eDestKeyOnly;
    invalidate.virtualKeyLadderID    = BCMD_VKL_KeyRam_eMax;
    invalidate.bInvalidateAllEntries = true;
    invalidate.keyDestBlckType       = BCMD_KeyDestBlockType_eMem2Mem;
    invalidate.unKeySlotNum          = pM2mKeySlot->keySlotNum;
    if( ( errCode = BHSM_InvalidateKey( hHsm, &invalidate ) ) !=  BERR_SUCCESS )
    {
        BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    pKeyslot->bIsUsed = false;

    BDBG_LEAVE( BHSM_FreeM2MKeySlot );
    return errCode;
}
#endif /* BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0) */




/* DEPRECATED, use BHSM_SetKeySlotOwnership */
BERR_Code BHSM_SetKeySlotOwner(
            BHSM_Handle          hHsm,
            unsigned             keySlotNumber,
            BCMD_XptSecKeySlot_e keySlotType,
            BHSM_KeySlotOwner_e  owner
)
{
#if BHSM_SUPPORT_KEYSLOT_OWNERSHIP
    BERR_Code rc;
    BHSM_KeySlotOwnership_t config;

    BKNI_Memset( &config, 0, sizeof(config) );

    config.keySlotNumber = keySlotNumber;
    config.keySlotType   = keySlotType;
    config.owner         = owner;

    BDBG_WRN(("Function BHSM_SetKeySlotOwner is deprecated. Use BHSM_SetKeySlotOwnership"));

    rc = BHSM_SetKeySlotOwnership( hHsm, &config );

    return rc;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( keySlotNumber );
    BSTD_UNUSED( keySlotType );
    BSTD_UNUSED( owner );
    return BERR_NOT_SUPPORTED;
#endif
}



bool isKeySlotOwershipSupported( BHSM_P_Handle  *hHsm )
{
 #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BSTD_UNUSED( hHsm );
    return true;
 #elif BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(3,0)
    if( hHsm->firmwareVersion.bseck.major >= 4 )
    {
        return true;
    }
 #else
    BSTD_UNUSED( hHsm );
 #endif

    return false;
}

/* DEPRECATED, use BHSM_GetKeySlotOwnership */
BERR_Code BHSM_GetKeySlotOwner( BHSM_Handle           hHsm,
                                unsigned              keySlotNumber,
                                BCMD_XptSecKeySlot_e  keySlotType,
                                BHSM_KeySlotOwner_e  *pOwner )
{
#if BHSM_SUPPORT_KEYSLOT_OWNERSHIP
    BERR_Code rc;
    BHSM_KeySlotOwnership_t config;

    BKNI_Memset( &config, 0, sizeof(config) );

    config.keySlotNumber = keySlotNumber;
    config.keySlotType   = keySlotType;

    BDBG_WRN(("Function BHSM_GetKeySlotOwner is deprecated. Use BHSM_GetKeySlotOwnership"));

    rc = BHSM_GetKeySlotOwnership( hHsm, &config );

    *pOwner = config.owner;

    return rc;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( keySlotNumber );
    BSTD_UNUSED( keySlotType );
    BSTD_UNUSED( pOwner );
    return BERR_NOT_SUPPORTED;
#endif
}


/* BHSM_SetKeySlotOwnership sets the ownership of a particular keyslot */
BERR_Code BHSM_SetKeySlotOwnership(
                                    BHSM_Handle             hHsm,
                                    BHSM_KeySlotOwnership_t *pConfig
                                  )
{
#if BHSM_SUPPORT_KEYSLOT_OWNERSHIP
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             status = 0;

    BDBG_ENTER( BHSM_SetKeySlotOwnership );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pConfig == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( isKeySlotOwershipSupported( hHsm ) == false )
    {
        /* assume ownership was set. */
        return BERR_SUCCESS;
    }

    if( isKeySlotTypeZzyzx( pConfig->keySlotType ) == true )
    {
        /* BSP COMMAND not supported for Zzyzz CA. Ignore */
        return BERR_SUCCESS;
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eAllocateKeySlot, &header );

    BHSM_BspMsg_Pack8 ( hMsg, BCMD_AllocateKeySlot_InCmd_eKeySlotType, pConfig->keySlotType );
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_AllocateKeySlot_InCmd_eKeySlotNumber, pConfig->keySlotNumber );
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_AllocateKeySlot_InCmd_eSetKeySlotOwnership, pConfig->owner );
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_AllocateKeySlot_InCmd_eOwnershipAction, BCMD_HostSage_KeySlotAction_SubCmd_eSetOwnership );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        rc = BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_AllocateKeySlot_OutCmd_eStatus, &status );  /* check status */
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error type[%d] num[%d]", __FUNCTION__, status, pConfig->keySlotType, pConfig->keySlotNumber ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE(BHSM_SetKeySlotOwnership);
    return rc;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pConfig );
    return BERR_NOT_SUPPORTED;
#endif
}

/* BHSM_GetKeySlotOwnership returns the ownership of a keyslot */
BERR_Code BHSM_GetKeySlotOwnership( BHSM_Handle               hHsm,
                                    BHSM_KeySlotOwnership_t  *pConfig
                                  )
{
#if BHSM_SUPPORT_KEYSLOT_OWNERSHIP
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             status = 0;
    uint8_t             owner = 0;

    BDBG_ENTER( BHSM_GetKeySlotOwnership );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pConfig == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( isKeySlotOwershipSupported( hHsm ) == false )
    {
        /* assume ownership is shared. */
        pConfig->owner = BCMD_HostSage_KeySlotAllocation_eSHARED;
        return BERR_SUCCESS;
    }

    /* compile time verify direct mapping between BHSM and BCMD ownership types */
    BDBG_CASSERT( (int)BHSM_KeySlotOwner_eFREE   == (int)BCMD_HostSage_KeySlotAllocation_eFREE  );
    BDBG_CASSERT( (int)BHSM_KeySlotOwner_eSHARED == (int)BCMD_HostSage_KeySlotAllocation_eSHARED);
    BDBG_CASSERT( (int)BHSM_KeySlotOwner_eSAGE   == (int)BCMD_HostSage_KeySlotAllocation_eSAGE  );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( isKeySlotTypeZzyzx( pConfig->keySlotType ) == true )
    {
        /* COMMAND not supported for Zzyzx CA */
        pConfig->owner = BCMD_HostSage_KeySlotAllocation_eFREE;
        return BERR_SUCCESS;
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eAllocateKeySlot, &header );

    BHSM_BspMsg_Pack8 ( hMsg, BCMD_AllocateKeySlot_InCmd_eKeySlotType, pConfig->keySlotType );
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_AllocateKeySlot_InCmd_eKeySlotNumber, pConfig->keySlotNumber );
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_AllocateKeySlot_InCmd_eOwnershipAction, BCMD_HostSage_KeySlotAction_SubCmd_eOwnership_Query );

    rc = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( rc != BERR_SUCCESS )
    {
        rc = BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_AllocateKeySlot_OutCmd_eStatus, &status ); /* check status */
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", __FUNCTION__, status ));
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_AllocateKeySlot_OutCmd_eKeySlotOwnership, &owner );

    pConfig->owner = owner; /* there is a verified direct mapping between BCMD and BHSM ownership values. */

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_GetKeySlotOwnership );
    return rc;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pConfig );
    return BERR_NOT_SUPPORTED;
#endif
}


BERR_Code BHSM_LoadRouteUserKey (
        BHSM_Handle                  hHsm,
        BHSM_LoadRouteUserKeyIO_t   *inoutp_loadRouteUserKeyIO
)
{
    BERR_Code                       errCode = BERR_SUCCESS;
    unsigned                        keyOffset = 0;
    unsigned                        keySize = 0;  /* sizeof key in 4 byte resolution */
    BHSM_P_XPTKeySlotAlgorithm_t    *pXPTKeySlotAlgorithm = NULL;
    #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_P_M2MKeySlotAlgorithm_t    *pM2MKeySlotAlgorithm = NULL;
    #endif
    BHSM_P_CAKeySlotInfo_t          *pKeyslot = NULL;
    #if HSM_IS_ASKM_28NM_ZEUS_4_1
    unsigned int                    xptCntrlWordHi;
    unsigned int                    xptCntrlWordLo;
    #endif
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;
    #if HSM_IS_ASKM_40NM_ZEUS_3_0
    unsigned extKsNum = 0; /*external keyslot number*/
    #endif

    BDBG_ENTER( BHSM_LoadRouteUserKey );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( inoutp_loadRouteUserKeyIO == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( ( errCode = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_LOAD_ROUTE_USERKEY, &header );

    /* Get pointer to structure BHSM_P_KeySlotAlgorithm_t for this keyslot */
    switch (inoutp_loadRouteUserKeyIO->keyDestBlckType)
    {

        case BCMD_KeyDestBlockType_eMem2Mem :

       #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
        {
            unsigned unKeySlotNum;
            unKeySlotNum = inoutp_loadRouteUserKeyIO->unKeySlotNum;

            /* Parameter checking on unKeySlotNum */
            if( unKeySlotNum >= BCMD_MAX_M2M_KEY_SLOT )
            {
                errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            pM2MKeySlotAlgorithm = &(hHsm->aunM2MKeySlotInfo[unKeySlotNum].aKeySlotAlgorithm[inoutp_loadRouteUserKeyIO->keyDestEntryType]);

            if( pM2MKeySlotAlgorithm->configured == false )
            {
                errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );
                goto BHSM_P_DONE_LABEL;
            }
        }
        break;
       #endif

        case BCMD_KeyDestBlockType_eCA :
        case BCMD_KeyDestBlockType_eCPDescrambler :
        case BCMD_KeyDestBlockType_eCPScrambler :
        {

            if( ( errCode = BHSM_P_GetKeySlot( hHsm,
                                              &pKeyslot,
                                               inoutp_loadRouteUserKeyIO->caKeySlotType,
                                               inoutp_loadRouteUserKeyIO->unKeySlotNum ) ) != BERR_SUCCESS )
            {
                errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            pXPTKeySlotAlgorithm = &(pKeyslot->aKeySlotAlgorithm[inoutp_loadRouteUserKeyIO->keyDestEntryType]);

            BDBG_MSG(("%s  keySlot[%d] ks-type[%d] destType[%d]"  \
                                , __FUNCTION__ \
                                , inoutp_loadRouteUserKeyIO->unKeySlotNum \
                                , inoutp_loadRouteUserKeyIO->caKeySlotType \
                                , inoutp_loadRouteUserKeyIO->keyDestEntryType \
                                ));
            if( pXPTKeySlotAlgorithm->configured == false )
            {
                errCode = BERR_TRACE(BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR);
                goto BHSM_P_DONE_LABEL;
            }
        }break;

        default:
        {
            /* no checking for other types; no key slot associated */
        }break;
    }

    if( inoutp_loadRouteUserKeyIO->keySize.eKeySize > BCMD_KeySize_e192 )
    {
        errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Pack8 ( hMsg, BCMD_LoadUseKey_InCmd_eKeyLength, (uint16_t)inoutp_loadRouteUserKeyIO->keySize.eKeySize );

    keySize = (( inoutp_loadRouteUserKeyIO->keySize.eKeySize * 2 ) + 2) * 4;     /* size of algorithm in bytes*/
    keyOffset = (8 - (2*(inoutp_loadRouteUserKeyIO->keySize.eKeySize + 1))) * 4; /* offset of key in bytes */

    BHSM_BspMsg_PackArray(  hMsg,
                            BCMD_LoadUseKey_InCmd_eKeyData+keyOffset,   /* offset */
                            inoutp_loadRouteUserKeyIO->aucKeyData,      /* data   */
                            keySize                                     /* length */ );


    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eRouteKeyFlag, inoutp_loadRouteUserKeyIO->bIsRouteKeyRequired?1:0 );

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    /*  If M2M op, check to see if we use CPS or CPD */
    if (inoutp_loadRouteUserKeyIO->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem)
    {
        if( pKeyslot->bDescrambling )
        {
            inoutp_loadRouteUserKeyIO->keyDestBlckType = BCMD_KeyDestBlockType_eCPDescrambler;
        }
        else
        {
            inoutp_loadRouteUserKeyIO->keyDestBlckType = BCMD_KeyDestBlockType_eCPScrambler;
        }
    }
#endif

    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eBlkType, inoutp_loadRouteUserKeyIO->keyDestBlckType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eEntryType, inoutp_loadRouteUserKeyIO->keyDestEntryType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eIVType, inoutp_loadRouteUserKeyIO->keyDestIVType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeySlotType, inoutp_loadRouteUserKeyIO->caKeySlotType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeySlotNumber, inoutp_loadRouteUserKeyIO->unKeySlotNum );

#if HSM_IS_ASKM_40NM_ZEUS_3_0
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eSC01ModeWordMapping,inoutp_loadRouteUserKeyIO->SC01ModeMapping & 0xFF );
#endif
#if HSM_IS_ASKM_40NM_ZEUS_2_0
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eGPipeSC01Value, inoutp_loadRouteUserKeyIO->GpipeSC01Val & 0x03 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eRPipeSC01Value, inoutp_loadRouteUserKeyIO->RpipeSC01Val & 0x03 );
#endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeyMode, inoutp_loadRouteUserKeyIO->keyMode & 0xFF );

    /* At this point, M2M DMA blockType is already set to either CPD or CPS */
    if ((inoutp_loadRouteUserKeyIO->keyDestBlckType == BCMD_KeyDestBlockType_eCA)            ||
        (inoutp_loadRouteUserKeyIO->keyDestBlckType == BCMD_KeyDestBlockType_eCPDescrambler) ||
        (inoutp_loadRouteUserKeyIO->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler)     )
    {
#if HSM_IS_ASKM_28NM_ZEUS_4_1
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord0, pKeyslot->ulGlobalControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord1, pKeyslot->ulGlobalControlWordLo );

        if( inoutp_loadRouteUserKeyIO->keyDestBlckType == BCMD_KeyDestBlockType_eCA )
        {
            xptCntrlWordHi = pXPTKeySlotAlgorithm->ulCAControlWordHi;
            xptCntrlWordLo = pXPTKeySlotAlgorithm->ulCAControlWordLo;
        }
        else if( inoutp_loadRouteUserKeyIO->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler )
        {
            xptCntrlWordHi = pXPTKeySlotAlgorithm->ulCPSControlWordHi;
            xptCntrlWordLo = pXPTKeySlotAlgorithm->ulCPSControlWordLo;
        }
        else
        {
            xptCntrlWordHi = pXPTKeySlotAlgorithm->ulCPDControlWordHi;
            xptCntrlWordLo = pXPTKeySlotAlgorithm->ulCPDControlWordLo;
        }

        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord2, xptCntrlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord3, xptCntrlWordLo );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord4, 0 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord5, 0 );

#else
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord0, pXPTKeySlotAlgorithm->ulCAControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord1, pXPTKeySlotAlgorithm->ulCAControlWordLo );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord2, pXPTKeySlotAlgorithm->ulCPSControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord3, pXPTKeySlotAlgorithm->ulCPSControlWordLo );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord4, pXPTKeySlotAlgorithm->ulCPDControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord5, pXPTKeySlotAlgorithm->ulCPDControlWordLo );

#endif

#if HSM_IS_ASKM_40NM_ZEUS_3_0
        if( pXPTKeySlotAlgorithm->externalKeySlot.valid )    /* Set external Key Slot parameters.  */
        {

            extKsNum = pXPTKeySlotAlgorithm->externalKeySlot.slotNum;
            if( extKsNum >= BHSM_EXTERNAL_KEYSLOTS_MAX )
            {
                /* index into hHsm->externalKeySlotTable is too large  */
                BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            if( hHsm->externalKeySlotTable[extKsNum].key.valid )
            {
                BHSM_BspMsg_Pack16( hMsg, BCMD_LoadUseKey_InCmd_eExtKeyPtr, ( hHsm->externalKeySlotTable[extKsNum].slotPtr +
                                                                              hHsm->externalKeySlotTable[extKsNum].key.offset ) );
            }

            if( hHsm->externalKeySlotTable[extKsNum].iv.valid )
            {
                BHSM_BspMsg_Pack16( hMsg, BCMD_LoadUseKey_InCmd_eExtIVPtr,  ( hHsm->externalKeySlotTable[extKsNum].slotPtr +
                                                                              hHsm->externalKeySlotTable[extKsNum].iv.offset ) );
            }
        }
#endif
    }

#if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
    else if (inoutp_loadRouteUserKeyIO->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem)
    {
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord0, pM2MKeySlotAlgorithm->ulM2MControlWordHi );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord1, pM2MKeySlotAlgorithm->ulM2MControlWordLo );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord2, 0 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord3, 0 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord4, 0 );
        BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord5, 0 );

    #if HSM_IS_ASKM_40NM_ZEUS_3_0
        /* BHSM_BspMsg_Pack16( hMsg, BCMD_LoadUseKey_InCmd_eExtKeyPtr, pM2MKeySlotAlgorithm->); */
        /* BHSM_BspMsg_Pack16( hMsg, BCMD_LoadUseKey_InCmd_eExtIVPtr, pM2MKeySlotAlgorithm->);*/
    #endif  /*HSM_IS_ASKM_40NM_ZEUS_3_0*/
    }
#endif /* BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0) */

    errCode = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    inoutp_loadRouteUserKeyIO->unStatus = (uint32_t)status;
    if( status != BERR_SUCCESS )
    {
        errCode = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", __FUNCTION__, status ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    if( hMsg )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE(BHSM_LoadRouteUserKey);
    return( errCode );
}


BERR_Code BHSM_InvalidateKey (
        BHSM_Handle                    hHsm,
        BHSM_InvalidateKeyIO_t        *inoutp_invalidateKeyIO
)
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status;
    #if BHSM_ZEUS_VERSION > BHSM_ZEUS_VERSION_CALC(4,1)
    unsigned offset = 0;
    #endif

    BDBG_ENTER( BHSM_InvalidateKey );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( inoutp_invalidateKeyIO == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    #if BHSM_ZEUS_VERSION > BHSM_ZEUS_VERSION_CALC(4,1)
    offset = (unsigned)hHsm->keySlotTypes[inoutp_invalidateKeyIO->caKeySlotType].ucKeySlotStartOffset;

    if( inoutp_invalidateKeyIO->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem )
    {
        if( hHsm->pKeySlot[(offset+(inoutp_invalidateKeyIO->unKeySlotNum))]->bDescrambling )
        {
            inoutp_invalidateKeyIO->keyDestBlckType = BCMD_KeyDestBlockType_eCPDescrambler;
        }
        else
        {
            inoutp_invalidateKeyIO->keyDestBlckType = BCMD_KeyDestBlockType_eCPScrambler;
        }
    }
    #endif

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_INVALIDATE_KEY, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeyFlag, inoutp_invalidateKeyIO->invalidKeyType );
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    if( inoutp_invalidateKeyIO->bInvalidateAllEntries )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eIsAllKTS, 1 );
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
        BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eSetByPassKTS, inoutp_invalidateKeyIO->bypassConfiguraion );
       #endif
    }
   #endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeyLayer, inoutp_invalidateKeyIO->keyLayer );


	if ( inoutp_invalidateKeyIO->invalidKeyType == BCMD_InvalidateKey_Flag_eSrcKeyOnly
		 || inoutp_invalidateKeyIO->invalidKeyType == BCMD_InvalidateKey_Flag_eBoth )
	{

		BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eVKLID, BHSM_RemapVklId(inoutp_invalidateKeyIO->virtualKeyLadderID ));
	}

    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eBlkType, inoutp_invalidateKeyIO->keyDestBlckType );

   #if HSM_IS_ASKM_40NM_ZEUS_3_0
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eSC01ModeWordMapping, inoutp_invalidateKeyIO->SC01ModeMapping );
   #endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eEntryType, inoutp_invalidateKeyIO->keyDestEntryType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeySlotType, inoutp_invalidateKeyIO->caKeySlotType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeySlotNumber, inoutp_invalidateKeyIO->unKeySlotNum );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    inoutp_invalidateKeyIO->unStatus =  (uint32_t)status;
    if( status != 0 )
    {
        BDBG_ERR(("%s BSP error status[0x%02X]. block[%d] type[%d] number[%d]", __FUNCTION__, status,
                                            inoutp_invalidateKeyIO->keyDestBlckType,
                                            inoutp_invalidateKeyIO->caKeySlotType,
                                            inoutp_invalidateKeyIO->unKeySlotNum ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    if( hMsg )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE(BHSM_InvalidateKey);
    return rc;
}


#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
BERR_Code BHSM_ConfigAlgorithm (
        BHSM_Handle                hHsm,
        BHSM_ConfigAlgorithmIO_t  *pConfig
)
{
    BERR_Code       errCode = BERR_SUCCESS;
    uint32_t        unCryptoAlg = 0;
    unsigned char   offset = 0 ;
    uint32_t        xptCntrlWordHi = 0;
    uint32_t        xptCntrlWordLo = 0;
    BHSM_P_CAKeySlotInfo_t       *pKeyslot    = NULL;
    BHSM_P_XPTKeySlotAlgorithm_t *pKeyslotAlg = NULL;
    BHSM_InvalidateKeyIO_t  invalidateKeyIO;
    int x = 0;

    BDBG_ENTER( BHSM_ConfigAlgorithm );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pConfig == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if(   pConfig->keyDestBlckType != BCMD_KeyDestBlockType_eMem2Mem
         && pConfig->keyDestBlckType != BCMD_KeyDestBlockType_eCA
         && pConfig->keyDestBlckType != BCMD_KeyDestBlockType_eCPScrambler
         && pConfig->keyDestBlckType != BCMD_KeyDestBlockType_eCPDescrambler )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );    /* invalid block type specified.  */
    }

    if( ( errCode = BHSM_P_GetKeySlot(  hHsm,
                                       &pKeyslot,
                                        pConfig->caKeySlotType,
                                        pConfig->unKeySlotNum ) ) != BERR_SUCCESS )
    {
        errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
        goto BHSM_P_DONE_LABEL;
    }



    if( pConfig->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem )
    {
        /* Invalidate the keyslot. */
        BKNI_Memset( & invalidateKeyIO, 0, sizeof(invalidateKeyIO) );

        invalidateKeyIO.caKeySlotType         = pConfig->caKeySlotType;
        invalidateKeyIO.invalidKeyType        = BCMD_InvalidateKey_Flag_eDestKeyOnly;
        invalidateKeyIO.unKeySlotNum          = pConfig->unKeySlotNum;

        /* Invalid all the destKey including IV. */
        invalidateKeyIO.bInvalidateAllEntries = true;

        /* Only invalid destKey, so the VKL is not affacted, set VKLId as invalid. */
        invalidateKeyIO.virtualKeyLadderID = BCMD_VKL_KeyRam_eMax;

        if( (errCode = BHSM_InvalidateKey( hHsm, &invalidateKeyIO ) ) != BERR_SUCCESS )
        {
            errCode = BERR_TRACE( errCode );
            goto BHSM_P_DONE_LABEL;
        }

       #if BHSM_SUPPORT_KEYSLOT_OWNERSHIP
        if( hHsm->currentSettings.clientType == BHSM_ClientType_eSAGE )
        {
            BHSM_KeySlotOwnership_t ownership;

            /* On SAGE, restore key slot ownership to SAGE (BHSM_InvalidateKey() returns the ownership to FREE). */
            BKNI_Memset( &ownership, 0, sizeof(ownership) );
            ownership.keySlotNumber = pConfig->unKeySlotNum;
            ownership.keySlotType   = pConfig->caKeySlotType;
            ownership.owner         = BHSM_KeySlotOwner_eSAGE;

            if( ( errCode = BHSM_SetKeySlotOwnership( hHsm, &ownership ) ) != BERR_SUCCESS )
            {
                BERR_TRACE( errCode );
                goto BHSM_P_DONE_LABEL;
            }
        }
       #endif
    }


    unCryptoAlg = pConfig->cryptoAlg.xptSecAlg;
    xptCntrlWordLo =
        (unCryptoAlg                                                        << BHSM_CaModeShift_eCryptoAlgorithmShift) |
        (pConfig->cryptoAlg.cipherDVBCSA2Mode        << BHSM_CaModeShift_eCipherModeShift) |
        (pConfig->cryptoAlg.termCounterMode          << BHSM_CaModeShift_eTerminationModeShift) |
        (pConfig->cryptoAlg.IVModeCounterSize        << BHSM_CaModeShift_eIVModeShift) |
        (pConfig->cryptoAlg.solitaryMode             << BHSM_CaModeShift_eSolitaryTermShift) |
        ((pConfig->cryptoAlg.keyOffset & 0x7F)       << BHSM_CaModeShift_eKeyOffsetShift)    |
        ((pConfig->cryptoAlg.ivOffset & 0x7F)        << BHSM_CaModeShift_eIVOffsetShift);

    xptCntrlWordHi =
        (pConfig->cryptoAlg.bUseExtKey                   << BHSM_CaModeShift_eAllowExtKeyShift)     |
        (pConfig->cryptoAlg.bUseExtIV                    << BHSM_CaModeShift_eAllowExtIVShift)    |
        (pConfig->cryptoAlg.bRpipeEnable                 << BHSM_CaModeShift_eRpipeEnableShift) |
        (pConfig->cryptoAlg.bGpipeEnable                 << BHSM_CaModeShift_eGpipeEnableShift) |
        ((pConfig->cryptoAlg.RpipeSCVal & 0x3)           << BHSM_CaModeShift_eRpipeSCValShift)  |
        ((pConfig->cryptoAlg.GpipeSCVal & 0x3)           << BHSM_CaModeShift_eGpipeSCValShift)  |
    #if HSM_IS_ASKM_28NM_ZEUS_4_1
        (pConfig->cryptoAlg.MSCLengthSelect              << BHSM_CaModeShift_eMSCLengthSelectShift41) |
        (pConfig->cryptoAlg.customerType                 << BHSM_CaModeShift_eCustomerNoShift41)  |
        (pConfig->cryptoAlg.MACRegSelect                 << BHSM_CaModeShift_eMACRegSelectShift41)        |
        (pConfig->cryptoAlg.MACNonSecureRegRead          << BHSM_CaModeShift_eMACRegAllowNSRead)          |
        (pConfig->cryptoAlg.Multi2SysKeySelect           << BHSM_CaModeShift_eSysKeySelectShift41)        |
        (pConfig->cryptoAlg.Multi2RoundCount             << BHSM_CaModeShift_eRoundCountShift41)          |
        (pConfig->cryptoAlg.IdertoModEnable              << BHSM_CaModeShift_eIrModEnableShift41)         |
        ((pConfig->cryptoAlg.DVBCSA3dvbcsaVar & 0x1F)    << BHSM_CaModeShift_eNDVBCSA3DvbCsaVarShift41)   |
        ((pConfig->cryptoAlg.DVBCSA3permutation & 0x7)   << BHSM_CaModeShift_eNDVBCSA3PermShift41)        |
        (pConfig->cryptoAlg.DVBCSA3modXRC                << BHSM_CaModeShift_eNDVBCSA3ModXRCShift41)      |
        ((pConfig->cryptoAlg.DVBCSA2keyCtrl & 0x7)       << BHSM_CaModeShift_eNDVBCSA2KeyCtrlShift41)     |
        ((pConfig->cryptoAlg.DVBCSA2ivCtrl & 0x3)        << BHSM_CaModeShift_eNDVBCSA2IVCtrlShift41)      |
        (pConfig->cryptoAlg.EsModEnable                  << BHSM_CaModeShift_eESDVBCSA2ModEnShift41);
    #else  /* HSM_IS_ASKM_28NM_ZEUS_4_1 */
        (pConfig->cryptoAlg.bRpipeFromRregion            << BHSM_CaModeShift_eRpipeFromRregionShift) |
        (pConfig->cryptoAlg.bRpipeFromGregion            << BHSM_CaModeShift_eRpipeFromGregionShift) |
        (pConfig->cryptoAlg.bRpipeToRregion              << BHSM_CaModeShift_eRpipeToRregionShift) |
        (pConfig->cryptoAlg.bRpipeToGregion              << BHSM_CaModeShift_eRpipeToGregionShift) |
        (pConfig->cryptoAlg.bGpipeFromRregion            << BHSM_CaModeShift_eGpipeFromRregionShift) |
        (pConfig->cryptoAlg.bGpipeFromGregion            << BHSM_CaModeShift_eGpipeFromGregionShift) |
        (pConfig->cryptoAlg.bGpipeToRregion              << BHSM_CaModeShift_eGpipeToRregionShift) |
        (pConfig->cryptoAlg.bGpipeToGregion              << BHSM_CaModeShift_eGpipeToGregionShift) |
        (pConfig->cryptoAlg.MSCLengthSelect              << BHSM_CaModeShift_eMSCLengthSelectShift) |
        (pConfig->cryptoAlg.bEncryptBeforeRave           << BHSM_CaModeShift_eEncryptBeforeRaveShift) |
        (pConfig->cryptoAlg.customerType                 << BHSM_CaModeShift_eCustomerNoShift)        |
        (pConfig->cryptoAlg.MACRegSelect                 << BHSM_CaModeShift_eMACRegSelectShift)      |
        (pConfig->cryptoAlg.Multi2SysKeySelect           << BHSM_CaModeShift_eSysKeySelectShift)      |
        (pConfig->cryptoAlg.Multi2RoundCount             << BHSM_CaModeShift_eRoundCountShift)        |
        (pConfig->cryptoAlg.IdertoModEnable              << BHSM_CaModeShift_eIrModEnableShift)       |
        ((pConfig->cryptoAlg.DVBCSA3dvbcsaVar & 0x1F)    << BHSM_CaModeShift_eNDVBCSA3DvbCsaVarShift) |
        ((pConfig->cryptoAlg.DVBCSA3permutation & 0x7)   << BHSM_CaModeShift_eNDVBCSA3PermShift)      |
        (pConfig->cryptoAlg.DVBCSA3modXRC                << BHSM_CaModeShift_eNDVBCSA3ModXRCShift)    |
        ((pConfig->cryptoAlg.DVBCSA2keyCtrl & 0x7)       << BHSM_CaModeShift_eNDVBCSA2KeyCtrlShift)   |
        ((pConfig->cryptoAlg.DVBCSA2ivCtrl & 0x3)        << BHSM_CaModeShift_eNDVBCSA2IVCtrlShift)    |
        (pConfig->cryptoAlg.EsModEnable                  << BHSM_CaModeShift_eESDVBCSA2ModEnShift);
    #endif  /* HSM_IS_ASKM_28NM_ZEUS_4_1 */

    BDBG_MSG(("%s keySlot[%d] offset[%d] caKeySlotType[%d] keyDestBlckType[%d] keyDestEntryType[%d] xptCntrlWordLo = 0x%x, xptCntrlWordHi = 0x%x"
                                            , __FUNCTION__
                                            , pConfig->unKeySlotNum
                                            , offset
                                            , pConfig->caKeySlotType
                                            , pConfig->keyDestBlckType
                                            , pConfig->keyDestEntryType
                                            , xptCntrlWordLo
                                            , xptCntrlWordHi ));

    /* If this is for a M2M DMA key slot, try determining if we use CPS or CPD */
    if (pConfig->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem)
    {
        /* Some extra setting for M2M DMA to work  */
        xptCntrlWordHi |=   (1   << BHSM_CaModeShift_eGpipeFromGregionShift)    |
                            (1   << BHSM_CaModeShift_eGpipeFromRregionShift)    |
                            (1   << BHSM_CaModeShift_eGpipeToGregionShift)      |
                            (1   << BHSM_CaModeShift_eGpipeToRregionShift)      |
                            (1   << BHSM_CaModeShift_eRpipeFromRregionShift)    |
                            (1   << BHSM_CaModeShift_eRpipeFromGregionShift)    |
                            (1   << BHSM_CaModeShift_eRpipeToRregionShift)      |
                            (1   << BHSM_CaModeShift_eRpipeToGregionShift);
        if (pConfig->cryptoAlg.cryptoOp == BHSM_M2mAuthCtrl_eDescramble)
        {
            pKeyslot->bDescrambling = true;
            pConfig->keyDestBlckType = BCMD_KeyDestBlockType_eCPDescrambler;
        }
        else
        {
            pKeyslot->bDescrambling = false;
            pConfig->keyDestBlckType = BCMD_KeyDestBlockType_eCPScrambler;
        }
    }

    pKeyslotAlg = &(pKeyslot->aKeySlotAlgorithm[pConfig->keyDestEntryType]);
    /* store the control words according to CA, CP-S, or CP-D */
    if (pConfig->keyDestBlckType == BCMD_KeyDestBlockType_eCA)
    {
        pKeyslotAlg->ulCAControlWordHi = xptCntrlWordHi;
        pKeyslotAlg->ulCAControlWordLo = xptCntrlWordLo;

    }
    else if (pConfig->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler)
    {
        pKeyslotAlg->ulCPSControlWordHi = xptCntrlWordHi;
        pKeyslotAlg->ulCPSControlWordLo = xptCntrlWordLo;
    }
    else if (pConfig->keyDestBlckType == BCMD_KeyDestBlockType_eCPDescrambler)
    {
        pKeyslotAlg->ulCPDControlWordHi = xptCntrlWordHi;
        pKeyslotAlg->ulCPDControlWordLo = xptCntrlWordLo;
    }

    if( pConfig->cryptoAlg.bUseExtKey || pConfig->cryptoAlg.bUseExtIV )
    {

        if( pKeyslotAlg->externalKeySlot.valid == false )
        {
           /* allocate an external keyslot */
           for( x = 0; x < BHSM_EXTERNAL_KEYSLOTS_MAX; x++ )
           {
               if( hHsm->externalKeySlotTable[x].allocated == false )
               {
                    hHsm->externalKeySlotTable[x].allocated = true;  /* reserve the ext. key slot.  */
                    pKeyslotAlg->externalKeySlot.slotNum = x;    /* record the ext. key slot location */
                    pKeyslotAlg->externalKeySlot.valid = true;
                   break;
               }
           }
        }
        if( pKeyslotAlg->externalKeySlot.valid )
        {
            unsigned slotNum = pKeyslotAlg->externalKeySlot.slotNum;

            if( pKeyslotAlg->externalKeySlot.slotNum >= BHSM_EXTERNAL_KEYSLOTS_MAX )
            {
                errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
                goto BHSM_P_DONE_LABEL;
            }
            hHsm->externalKeySlotTable[slotNum].key.valid = pConfig->cryptoAlg.bUseExtKey;
            hHsm->externalKeySlotTable[slotNum].iv.valid  = pConfig->cryptoAlg.bUseExtIV;
        }

        BDBG_MSG(("ExternalKey SET ksNum[%d] ksType[%d] blkType[%d] polType[%d] x[%d]" , pConfig->unKeySlotNum
                            , pConfig->caKeySlotType, pConfig->keyDestBlckType, pConfig->keyDestEntryType, x ));

        if( x == BHSM_EXTERNAL_KEYSLOTS_MAX )
        {
            /* what have failed to reserve an external keyslot. */
            errCode = BERR_TRACE( BHSM_STATUS_RESOURCE_ALLOCATION_ERROR );
            goto BHSM_P_DONE_LABEL;
        }
    }

    pKeyslotAlg->configured = true;

BHSM_P_DONE_LABEL:

    BDBG_LEAVE( BHSM_ConfigAlgorithm );
    return errCode;
}


/* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0) */
BERR_Code BHSM_GetKeySlotConfigAlgorithm (
        BHSM_Handle                hHsm,
        BHSM_KeySlotInfo_t        *inoutp_KeySlotInfoIO
)
{
    BERR_Code            errCode              = BERR_SUCCESS;
    uint32_t            xptCntrlWordHi = 0;
    uint32_t            xptCntrlWordLo = 0;
    BHSM_P_CAKeySlotInfo_t *pKeyslot = NULL;

    BDBG_ENTER(BHSM_GetKeySlotConfigAlgorithm);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( inoutp_KeySlotInfoIO == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    /* For M2M case, unKeySlotNum is sufficient. */
    switch (inoutp_KeySlotInfoIO->keyDestBlckType)
    {
        case BCMD_KeyDestBlockType_eMem2Mem:
            /* Parameter checking on unKeySlotNum */
            /* Set the CA key slot type for M2M DMA key slot and fall through */
            /* inoutp_KeySlotInfoIO->caKeySlotType has to be set to the right key slot type for this M2M key slot */
            /*inoutp_KeySlotInfoIO->caKeySlotType = BCMD_XptSecKeySlot_eType0; */
        case BCMD_KeyDestBlockType_eCA:
        case BCMD_KeyDestBlockType_eCPScrambler:
        case BCMD_KeyDestBlockType_eCPDescrambler:

            if( ( errCode =  BHSM_P_GetKeySlot(  hHsm,
                                                &pKeyslot,
                                                 inoutp_KeySlotInfoIO->caKeySlotType,
                                                 inoutp_KeySlotInfoIO->unKeySlotNum ) ) != BERR_SUCCESS )
            {
                return BERR_TRACE( errCode );
            }

            /* Change keyDestBlckType for M2M DMA key slot */
            if (inoutp_KeySlotInfoIO->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem)
            {
                if (pKeyslot->bDescrambling)
                {
                    inoutp_KeySlotInfoIO->keyDestBlckType = BCMD_KeyDestBlockType_eCPDescrambler;
                }
                else
                {
                    inoutp_KeySlotInfoIO->keyDestBlckType = BCMD_KeyDestBlockType_eCPScrambler;
                }
            }
            /* retrieve the control words according to CA, CP-S, or CP-D */
            if (inoutp_KeySlotInfoIO->keyDestBlckType == BCMD_KeyDestBlockType_eCA)
            {
                xptCntrlWordHi = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCAControlWordHi;
                xptCntrlWordLo = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCAControlWordLo;

            }
            else if (inoutp_KeySlotInfoIO->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler)
            {
                xptCntrlWordHi = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCPSControlWordHi;
                xptCntrlWordLo = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCPSControlWordLo;
            }
            else if (inoutp_KeySlotInfoIO->keyDestBlckType == BCMD_KeyDestBlockType_eCPDescrambler)
            {
                xptCntrlWordHi = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCPDControlWordHi;
                xptCntrlWordLo = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCPDControlWordLo;
            }

            inoutp_KeySlotInfoIO->cryptoAlg.xptSecAlg           = (xptCntrlWordLo >> BHSM_CaModeShift_eCryptoAlgorithmShift) & 0x1F;
            inoutp_KeySlotInfoIO->cryptoAlg.cipherDVBCSA2Mode   = (xptCntrlWordLo >> BHSM_CaModeShift_eCipherModeShift) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.termCounterMode     = (xptCntrlWordLo >> BHSM_CaModeShift_eTerminationModeShift) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.IVModeCounterSize   = (xptCntrlWordLo >> BHSM_CaModeShift_eIVModeShift) & 0x03;
            inoutp_KeySlotInfoIO->cryptoAlg.solitaryMode        = (xptCntrlWordLo >> BHSM_CaModeShift_eSolitaryTermShift) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.RpipeSCVal          = (xptCntrlWordHi >> BHSM_CaModeShift_eRpipeSCValShift) & 0x03;
            inoutp_KeySlotInfoIO->cryptoAlg.GpipeSCVal          = (xptCntrlWordHi >> BHSM_CaModeShift_eGpipeSCValShift) & 0x03;

            BDBG_MSG(("xptCntrlWordLo = 0x%x, xptCntrlWordHi = 0x%x", xptCntrlWordLo, xptCntrlWordHi));

            break;

        default:
            errCode = BERR_TRACE((BHSM_STATUS_INPUT_PARM_ERR));
            goto BHSM_P_DONE_LABEL;

    }

BHSM_P_DONE_LABEL:

    BDBG_LEAVE(BHSM_GetKeySlotConfigAlgorithm);
    return errCode;
}


#else  /* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0) */
 /* less that Zeus 4.0 */
BERR_Code BHSM_ConfigAlgorithm (
        BHSM_Handle               hHsm,
        BHSM_ConfigAlgorithmIO_t *pConfigAlgorithm
)
{
    BERR_Code            errCode       = BERR_SUCCESS;
    uint32_t            unCryptoAlg   = 0;
    unsigned char       ucKeySlotStartOffset = 0 ;
    uint32_t            m2mCntrlWordHi = 0;
    uint32_t            m2mCntrlWordLo = 0;
    uint32_t            xptCntrlWordHi = 0;
    uint32_t            xptCntrlWordLo = 0;
    unsigned int        unKeySlotNum   = 0;
    BHSM_P_CAKeySlotInfo_t  *pKeyslot = NULL;

    BDBG_ENTER(BHSM_ConfigAlgorithm);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pConfigAlgorithm == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    unKeySlotNum = pConfigAlgorithm->unKeySlotNum;

    /* For M2M case, unKeySlotNum is sufficient. */
    switch (pConfigAlgorithm->keyDestBlckType)
    {
        case BCMD_KeyDestBlockType_eMem2Mem:
            /* Parameter checking on unKeySlotNum */
            BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
                (unKeySlotNum >= BCMD_MAX_M2M_KEY_SLOT) );

            #if BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(3,0)
            /* TerminationAESCounterKeyMode is the same as AES Counter mode. */
            /* AES Counter modes 0, 2 and 4 do not support M2M on Zeus 3.0. */
            if (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.m2mCipherMode == BCMD_CipherModeSelect_eCTR)
            {

                if ((pConfigAlgorithm->cryptoAlg.m2mCryptAlg.TerminationAESCounterKeyMode == 0) ||
                    (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.TerminationAESCounterKeyMode == 2) ||
                    (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.TerminationAESCounterKeyMode == 4))
                {
                    BDBG_ERR(("Not supported AES Counter Mode %d on Zeus 3.0 for M2M.", pConfigAlgorithm->cryptoAlg.m2mCryptAlg.TerminationAESCounterKeyMode));
                    errCode = BERR_TRACE( BHSM_NOT_SUPPORTED_ERR );
                    goto BHSM_P_DONE_LABEL;
                }
             }
            #endif

            unCryptoAlg = pConfigAlgorithm->cryptoAlg.m2mCryptAlg.m2mSecAlg;

            m2mCntrlWordLo =
                (unCryptoAlg                                                         << BHSM_M2mModeShift_eCryptoAlgorithmShift) |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.m2mCipherMode         << BHSM_M2mModeShift_eCipherModeSel) |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.TerminationAESCounterKeyMode << BHSM_M2mModeShift_eAESCounterTermModeShift) |
                #if  BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.counterSize         << BHSM_M2mModeShift_eCounterSizeSel) |
                #endif
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.IVModeSelect         << BHSM_M2mModeShift_eIVModeSel) |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.SolitarySelect      << BHSM_M2mModeShift_eSolitaryTermSel) |
                ((pConfigAlgorithm->cryptoAlg.m2mCryptAlg.keyOffset & 0x7F)  << BHSM_M2mModeShift_eKeyOffset)  |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.bUseExtKey          << BHSM_M2mModeShift_eUseExtKey)  |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.bUseExtIV             << BHSM_M2mModeShift_eUseExtIV);

            m2mCntrlWordHi =
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.ucAuthCtrl          << BHSM_M2mModeShift_eAuthCtrlShift)  |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.bDisallowGG         << BHSM_M2mModeShift_eDisallowGG) |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.bDisallowGR         << BHSM_M2mModeShift_eDisallowGR) |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.bDisallowRG         << BHSM_M2mModeShift_eDisallowRG) |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.bDisallowRR         << BHSM_M2mModeShift_eDisallowRR) |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.bEnableTimestamp     << BHSM_M2mModeShift_eEnableTimeStamp) |
                (pConfigAlgorithm->cryptoAlg.m2mCryptAlg.bMscCtrlSel         << BHSM_M2mModeShift_eMscCtrlSel);

            BDBG_MSG(("m2m Slot[%d] Entry[%d] Lo[%#x], Hi[%#x]",unKeySlotNum, pConfigAlgorithm->keyDestEntryType, (unsigned)m2mCntrlWordLo, (unsigned)m2mCntrlWordHi));

            /* Store m2mCntrlWordLo and m2mCntrlWordHi  */
            hHsm->aunM2MKeySlotInfo[unKeySlotNum].aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].ulM2MControlWordLo = m2mCntrlWordLo;
            hHsm->aunM2MKeySlotInfo[unKeySlotNum].aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].ulM2MControlWordHi = m2mCntrlWordHi;


            hHsm->aunM2MKeySlotInfo[unKeySlotNum].aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].configured = true;

            break;

        case BCMD_KeyDestBlockType_eCA:
        case BCMD_KeyDestBlockType_eCPScrambler:
        case BCMD_KeyDestBlockType_eCPDescrambler:

            ucKeySlotStartOffset = hHsm->keySlotTypes[pConfigAlgorithm->caKeySlotType].ucKeySlotStartOffset;
            unCryptoAlg = pConfigAlgorithm->cryptoAlg.caCryptAlg.caSecAlg;

            if( unKeySlotNum >= hHsm->keySlotTypes[pConfigAlgorithm->caKeySlotType].unKeySlotNum )
            {
                errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            if( ( pKeyslot = hHsm->pKeySlot[ ucKeySlotStartOffset + unKeySlotNum ] ) == NULL )
            {
                errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );  /* referenced keyslot that is out of range. */
                goto BHSM_P_DONE_LABEL;
            }

            xptCntrlWordLo =
                (unCryptoAlg                                                        << BHSM_CaModeShift_eCryptoAlgorithmShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.cipherDVBCSA2Mode   << BHSM_CaModeShift_eCipherModeShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.terminationMode     << BHSM_CaModeShift_eTerminationModeShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.IVMode                << BHSM_CaModeShift_eIVModeShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.solitaryMode        << BHSM_CaModeShift_eSolitaryTermShift) |
                ((pConfigAlgorithm->cryptoAlg.caCryptAlg.keyOffset & 0x7F)  << BHSM_CaModeShift_eKeyOffset)    |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bUseExtKey          << BHSM_CaModeShift_eUseExtKeyShift)  |
                ((pConfigAlgorithm->cryptoAlg.caCryptAlg.ivOffset & 0x7F)   << BHSM_CaModeShift_eIVOffsetShift)   |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bUseExtIV           << BHSM_CaModeShift_eUseExtIVShift);

            xptCntrlWordHi =
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bRestrictEnable         << BHSM_CaModeShift_eRestrEnShift)     |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bGlobalEnable            << BHSM_CaModeShift_eGlobalEnShift)    |
                ((pConfigAlgorithm->cryptoAlg.caCryptAlg.restrSCVal  & 0x3)     << BHSM_CaModeShift_eRestrScValShift)  |
                ((pConfigAlgorithm->cryptoAlg.caCryptAlg.globalSCVal & 0x3)     << BHSM_CaModeShift_eGlobalScValShift)  |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bRestrictDropPktEnabled << BHSM_CaModeShift_eRestrictDropPktCtrlShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bGlobalDropPktEnabled   << BHSM_CaModeShift_eGlobalDropPktCtrlShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.ucMulti2KeySelect        << BHSM_CaModeShift_eMscSelMulti2SelShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.ucMSCLengthSelect       << BHSM_CaModeShift_eMscSelMulti2SelShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bGlobalRegOverride      << BHSM_CaModeShift_eGlobalRegOverrideShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bDropRregionPackets     << BHSM_CaModeShift_eDropFromRSourceShift)   |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bEncryptBeforeRave        << BHSM_CaModeShift_eEncryptBeforeRaveShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.customerType            << BHSM_CaModeShift_eCustomerNoShift)        |
                ((pConfigAlgorithm->cryptoAlg.caCryptAlg.DVBCSA3dvbcsaVar & 0x1F)  << BHSM_CaModeShift_NDVBCSA3DvbCsaVarShift)|
                ((pConfigAlgorithm->cryptoAlg.caCryptAlg.DVBCSA3permutation & 0x7) << BHSM_CaModeShift_NDVBCSA3PermShift)     |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.DVBCSA3modXRC           << BHSM_CaModeShift_NDVBCSA3ModXRCShift)      |
                ((pConfigAlgorithm->cryptoAlg.caCryptAlg.DVBCSA2keyCtrl & 0x7)  << BHSM_CaModeShift_NDVBCSA2KeyCtrlShift)     |
                ((pConfigAlgorithm->cryptoAlg.caCryptAlg.DVBCSA2ivCtrl & 0x3)   << BHSM_CaModeShift_NDVBCSA2IVCtrlShift)      |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.DVBCSA2modEnabled       << BHSM_CaModeShift_ESDVBCSA2ModEnShift)  |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bGpipePackets2Rregion   << BHSM_CaModeShift_GpipeForceRestrictedShift) |
                (pConfigAlgorithm->cryptoAlg.caCryptAlg.bRpipePackets2Rregion   << BHSM_CaModeShift_RpipeForceRestrictedShift);


            BDBG_MSG(("xptCntrlWordLo = %#x, xptCntrlWordHi = %#x", (unsigned)xptCntrlWordLo, (unsigned)xptCntrlWordHi));

            /* store the control words according to CA, CP-S, or CP-D */
            if (pConfigAlgorithm->keyDestBlckType == BCMD_KeyDestBlockType_eCA)
            {
                pKeyslot->aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].ulCAControlWordHi = xptCntrlWordHi;
                pKeyslot->aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].ulCAControlWordLo = xptCntrlWordLo;

            }
            else if (pConfigAlgorithm->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler)
            {
                pKeyslot->aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].ulCPSControlWordHi = xptCntrlWordHi;
                pKeyslot->aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].ulCPSControlWordLo = xptCntrlWordLo;
            }
            else if (pConfigAlgorithm->keyDestBlckType == BCMD_KeyDestBlockType_eCPDescrambler)
            {
                pKeyslot->aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].ulCPDControlWordHi = xptCntrlWordHi;
                pKeyslot->aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].ulCPDControlWordLo = xptCntrlWordLo;
            }

            pKeyslot->aKeySlotAlgorithm[pConfigAlgorithm->keyDestEntryType].configured = true;

            break;

        default:
            errCode = BERR_TRACE((BHSM_STATUS_INPUT_PARM_ERR));
            goto BHSM_P_DONE_LABEL;

    }

BHSM_P_DONE_LABEL:

    BDBG_LEAVE(BHSM_ConfigAlgorithm);
    return( errCode );
}


/* not BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0) */
BERR_Code BHSM_GetKeySlotConfigAlgorithm (
        BHSM_Handle                hHsm,
        BHSM_KeySlotInfo_t        *inoutp_KeySlotInfoIO
)
{
    BERR_Code           errCode              = BERR_SUCCESS;
    unsigned char       ucKeySlotStartOffset = 0 ;
    uint32_t            m2mCntrlWordHi = 0;
    uint32_t            m2mCntrlWordLo = 0;
    uint32_t            xptCntrlWordHi = 0;
    uint32_t            xptCntrlWordLo = 0;
    unsigned            unKeySlotNum   = 0;
    BHSM_P_CAKeySlotInfo_t *pKeyslot = NULL;

    BDBG_ENTER(BHSM_GetKeySlotConfigAlgorithm);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( inoutp_KeySlotInfoIO == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    unKeySlotNum  = inoutp_KeySlotInfoIO->unKeySlotNum;

    switch( inoutp_KeySlotInfoIO->keyDestBlckType )
    {
        case BCMD_KeyDestBlockType_eMem2Mem:
            /* Parameter checking on unKeySlotNum */
            BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
                (unKeySlotNum >= BCMD_MAX_M2M_KEY_SLOT) );

            /* Retrieve m2mCntrlWordLo and m2mCntrlWordHi    */
            m2mCntrlWordLo = hHsm->aunM2MKeySlotInfo[unKeySlotNum].aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulM2MControlWordLo;
            m2mCntrlWordHi = hHsm->aunM2MKeySlotInfo[unKeySlotNum].aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulM2MControlWordHi;
            inoutp_KeySlotInfoIO->cryptoAlg.m2mCryptAlg.m2mSecAlg     = (m2mCntrlWordLo >> BHSM_M2mModeShift_eCryptoAlgorithmShift) & 0x1F;
            inoutp_KeySlotInfoIO->cryptoAlg.m2mCryptAlg.m2mCipherMode = (m2mCntrlWordLo >> BHSM_M2mModeShift_eCipherModeSel) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.m2mCryptAlg.TerminationAESCounterKeyMode = (m2mCntrlWordLo >> BHSM_M2mModeShift_eAESCounterTermModeShift) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.m2mCryptAlg.counterSize   = (m2mCntrlWordLo >> BHSM_M2mModeShift_eAESCounterTermModeShift) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.m2mCryptAlg.IVModeSelect  = (m2mCntrlWordLo >> BHSM_M2mModeShift_eIVModeSel) & 0x03;
            inoutp_KeySlotInfoIO->cryptoAlg.m2mCryptAlg.SolitarySelect = (m2mCntrlWordLo >> BHSM_M2mModeShift_eSolitaryTermSel) & 0x07;

            BDBG_MSG(("m2mCntrlWordLo = %#x, m2mCntrlWordHi = %#x", (unsigned)m2mCntrlWordLo, (unsigned)m2mCntrlWordHi));

            break;

        case BCMD_KeyDestBlockType_eCA:
        case BCMD_KeyDestBlockType_eCPScrambler:
        case BCMD_KeyDestBlockType_eCPDescrambler:

            ucKeySlotStartOffset = hHsm->keySlotTypes[inoutp_KeySlotInfoIO->caKeySlotType].ucKeySlotStartOffset;

            if( unKeySlotNum >= hHsm->keySlotTypes[inoutp_KeySlotInfoIO->caKeySlotType].unKeySlotNum )
            {
                errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            if( ( pKeyslot = hHsm->pKeySlot[(ucKeySlotStartOffset+unKeySlotNum)] ) == NULL )
            {
                errCode = BERR_TRACE( BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR );
                goto BHSM_P_DONE_LABEL;
            }

            /* retrieve the control words according to CA, CP-S, or CP-D */
            if (inoutp_KeySlotInfoIO->keyDestBlckType == BCMD_KeyDestBlockType_eCA)
            {
                xptCntrlWordHi = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCAControlWordHi;
                xptCntrlWordLo = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCAControlWordLo;

            }
            else if (inoutp_KeySlotInfoIO->keyDestBlckType == BCMD_KeyDestBlockType_eCPScrambler)
            {
                xptCntrlWordHi = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCPSControlWordHi;
                xptCntrlWordLo = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCPSControlWordLo;
            }
            else if (inoutp_KeySlotInfoIO->keyDestBlckType == BCMD_KeyDestBlockType_eCPDescrambler)
            {
                xptCntrlWordHi = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCPDControlWordHi;
                xptCntrlWordLo = pKeyslot->aKeySlotAlgorithm[inoutp_KeySlotInfoIO->keyDestEntryType].ulCPDControlWordLo;
            }

            inoutp_KeySlotInfoIO->cryptoAlg.caCryptAlg.caSecAlg          = (xptCntrlWordLo >> BHSM_CaModeShift_eCryptoAlgorithmShift) & 0x1F;
            inoutp_KeySlotInfoIO->cryptoAlg.caCryptAlg.cipherDVBCSA2Mode = (xptCntrlWordLo >> BHSM_CaModeShift_eCipherModeShift) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.caCryptAlg.terminationMode   = (xptCntrlWordLo >> BHSM_CaModeShift_eTerminationModeShift) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.caCryptAlg.IVMode            = (xptCntrlWordLo >> BHSM_CaModeShift_eIVModeShift) & 0x03;
            inoutp_KeySlotInfoIO->cryptoAlg.caCryptAlg.solitaryMode      = (xptCntrlWordLo >> BHSM_CaModeShift_eSolitaryTermShift) & 0x07;
            inoutp_KeySlotInfoIO->cryptoAlg.caCryptAlg.restrSCVal        = (xptCntrlWordHi >> BHSM_CaModeShift_eRestrScValShift) & 0x03;
            inoutp_KeySlotInfoIO->cryptoAlg.caCryptAlg.globalSCVal       = (xptCntrlWordHi >> BHSM_CaModeShift_eGlobalScValShift) & 0x03;

            BDBG_MSG(("xptCntrlWordLo = %#x, xptCntrlWordHi = %#x", (unsigned)xptCntrlWordLo, (unsigned)xptCntrlWordHi));

            break;

        default:
            errCode = BERR_TRACE((BHSM_STATUS_INPUT_PARM_ERR));
            goto BHSM_P_DONE_LABEL;

    }

BHSM_P_DONE_LABEL:

    BDBG_LEAVE(BHSM_GetKeySlotConfigAlgorithm);
    return( errCode );

}


#endif /* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0) */



BERR_Code BHSM_GetExternalKeyIdentifier(
            BHSM_Handle hHsm,
            BHSM_KeyLocation_t *pKeyLocation,
            BHSM_ExternalKeyIdentifier_t *pExtKey )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    BERR_Code rc = BERR_SUCCESS;
    BHSM_ExternalKeySlot_t *pExtKeySlot;
    BHSM_P_XPTKeySlotAlgorithm_t  *pSlotAlgorithm = NULL;
    BHSM_P_CAKeySlotInfo_t *pKeyslot = NULL;

    BDBG_ENTER( BHSM_GetExternalKeyIdentifier );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pKeyLocation == NULL || pExtKey == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( ( rc = BHSM_P_GetKeySlot( hHsm,
                                 &pKeyslot,
                                  pKeyLocation->caKeySlotType,
                                  pKeyLocation->unKeySlotNum ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    if(  pKeyLocation->keyDestEntryType >= BCMD_KeyDestEntryType_eMax )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    BKNI_Memset( pExtKey, 0, sizeof(*pExtKey) );

    if( pKeyLocation->keyDestBlckType == BCMD_KeyDestBlockType_eMem2Mem )
    {

        if( pKeyslot->bDescrambling )
        {
            pKeyLocation->keyDestBlckType = BCMD_KeyDestBlockType_eCPDescrambler;
        }
        else
        {
            pKeyLocation->keyDestBlckType = BCMD_KeyDestBlockType_eCPScrambler;
        }
    }

    BDBG_MSG(("ExternalKey GET ksNum[%d] ksType[%d] blkType[%d] polType[%d]" , pKeyLocation->unKeySlotNum
                                                                               , pKeyLocation->caKeySlotType
                                                                               , pKeyLocation->keyDestBlckType
                                                                               , pKeyLocation->keyDestEntryType  ));

    pSlotAlgorithm = &(pKeyslot->aKeySlotAlgorithm[pKeyLocation->keyDestEntryType]);

    /* load the external key  slot details. */
    if( pSlotAlgorithm->externalKeySlot.valid )
    {

        if( pSlotAlgorithm->externalKeySlot.slotNum >= BHSM_EXTERNAL_KEYSLOTS_MAX )
        {
            /* invalid offset into external keyslot table.  */
            return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
        }

        pExtKeySlot = &hHsm->externalKeySlotTable[pSlotAlgorithm->externalKeySlot.slotNum];

        pExtKey->slotIndex = pExtKeySlot->slotPtr;

        if( pExtKeySlot->key.valid )
        {
            pExtKey->key.valid = true;
            pExtKey->key.offset = pExtKeySlot->key.offset;
        }

        if( pExtKeySlot->iv.valid )
        {
            pExtKey->iv.valid = true;
            pExtKey->iv.offset= pExtKeySlot->iv.offset;
        }
    }

    BDBG_LEAVE( BHSM_GetExternalKeyIdentifier );
    return rc;

#else /* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1) */
        BSTD_UNUSED( hHsm ) ;
        BSTD_UNUSED( pKeyLocation );
        BSTD_UNUSED( pExtKey ) ;
        return BERR_TRACE( BHSM_NOT_SUPPORTED_ERR );
#endif
}

BERR_Code BHSM_ConfigKeySlotIDData (
        BHSM_Handle                        hHsm,
        BHSM_ConfigKeySlotIDDataIO_t    *inoutp_configKeySlotIDDataIO
)
{
    BERR_Code          errCode              = BERR_SUCCESS;
    BHSM_P_AskmData_t *pAskmData;
    BHSM_P_CAKeySlotInfo_t *pKeyslot = NULL;

    BDBG_ENTER(BHSM_ConfigKeySlotIDData);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( inoutp_configKeySlotIDDataIO == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    switch (inoutp_configKeySlotIDDataIO->keyDestBlckType)
    {
        case BCMD_KeyDestBlockType_eMem2Mem:
    #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
        {
            unsigned unKeySlotNum = inoutp_configKeySlotIDDataIO->unKeySlotNum;

            if( unKeySlotNum >= BCMD_MAX_M2M_KEY_SLOT )
            {
                BDBG_ERR(("%s unKeySlotNum[%d]", __FUNCTION__, unKeySlotNum ));
                errCode = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
                goto BHSM_P_DONE_LABEL;
            }
            pAskmData = &(hHsm->aunM2MKeySlotInfo[unKeySlotNum].askmData[inoutp_configKeySlotIDDataIO->keyDestEntryType]);
        }
        break;
    #endif
        case BCMD_KeyDestBlockType_eCA:
        case BCMD_KeyDestBlockType_eCPScrambler:
        case BCMD_KeyDestBlockType_eCPDescrambler:

            if( ( errCode =  BHSM_P_GetKeySlot( hHsm,
                                               &pKeyslot,
                                                inoutp_configKeySlotIDDataIO->caKeySlotType,
                                                inoutp_configKeySlotIDDataIO->unKeySlotNum ) ) != BERR_SUCCESS )
            {
                return BERR_TRACE( errCode );
            }

            pAskmData = &( pKeyslot->askmData[inoutp_configKeySlotIDDataIO->keyDestEntryType] );
            break;

        default:
            errCode = BERR_TRACE((BHSM_STATUS_INPUT_PARM_ERR));
            goto BHSM_P_DONE_LABEL;

    }

    /* Store the ID Data now */
    pAskmData->ulCAVendorID         = inoutp_configKeySlotIDDataIO->CAVendorID;
    pAskmData->ulModuleID           = inoutp_configKeySlotIDDataIO->ModuleID;
    pAskmData->maskKeySelect        = inoutp_configKeySlotIDDataIO->key2Select;
    pAskmData->ulSTBOwnerIDSelect   = inoutp_configKeySlotIDDataIO->STBOwnerIDSelect;

BHSM_P_DONE_LABEL:

    BDBG_LEAVE(BHSM_ConfigKeySlotIDData);
    return( errCode );

}

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)

BERR_Code BHSM_ConfigKeySlotGlobalCntrlWord (
    BHSM_Handle                        hHsm,
    BHSM_KeySlotGlobalCntrlWord_t    *inoutp_configKeySlotGlobalCntrlWord
)
{
    BERR_Code               errCode              = BERR_SUCCESS;
    BHSM_P_CAKeySlotInfo_t  *pKeyslot;

    BDBG_ENTER(BHSM_ConfigKeySlotIDData);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( inoutp_configKeySlotGlobalCntrlWord == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( ( errCode = BHSM_P_GetKeySlot( hHsm,
                                      &pKeyslot,
                                       inoutp_configKeySlotGlobalCntrlWord->caKeySlotType,
                                       inoutp_configKeySlotGlobalCntrlWord->unKeySlotNum ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( errCode );
    }

    /* Store Global control word settings  now */
    pKeyslot->ulGlobalControlWordHi = 0;
    pKeyslot->ulGlobalControlWordLo = ( inoutp_configKeySlotGlobalCntrlWord->inputRegion       << BHSM_GlobalModeShift_eInputRegion )       |
                                      ( inoutp_configKeySlotGlobalCntrlWord->RpipeOutput       << BHSM_GlobalModeShift_eRpipeOutputRegion ) |
                                      ( inoutp_configKeySlotGlobalCntrlWord->GpipeOutput       << BHSM_GlobalModeShift_eGPipeOutputRegion ) |
                                      ( inoutp_configKeySlotGlobalCntrlWord->encryptBeforeRAVE << BHSM_GlobalModeShift_eEncryptBeforeRave );

    BDBG_LEAVE(BHSM_ConfigKeySlotGlobalCntrlWord);
    return errCode ;
}
#endif /* BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0) */




BERR_Code BHSM_ConfigMulti2 (
        BHSM_Handle                hHsm,
        BHSM_ConfigMulti2IO_t     *pConfigMulti2
)
{
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t  status = 0;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_ConfigMulti2 );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pConfigMulti2 == NULL )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( pConfigMulti2->ucSysKeyDest >= BCMD_MULTI2_MAXSYSKEY )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc ); /* failed create a BSP message  */
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eCONFIG_MULTI2, &header );

    BHSM_BspMsg_Pack32( hMsg, BCMD_Multi2_InCmdCfg_eRoundCount, pConfigMulti2->ucMulti2RndCnt );
    BHSM_BspMsg_PackArray( hMsg, BCMD_Multi2_InCmdCfg_eSystemKeys, pConfigMulti2->aucMulti2SysKey, BHSM_MULTI2_SYS_KEY_SIZE );
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_Multi2_InCmdCfg_eWhichSysKey, pConfigMulti2->ucSysKeyDest );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pConfigMulti2->unStatus = status;
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", __FUNCTION__, status ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    if( hMsg )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_ConfigMulti2 );
    return rc;
}


BERR_Code BHSM_ConfigPidChannelToDefaultKeySlot (
        BHSM_Handle                            hHsm,
        BHSM_ConfigPidKeyPointerTableIO_t    *pPidChannelConf
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BHSM_ConfigPidChannelToDefaultKeySlot);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pPidChannelConf == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    pPidChannelConf->bResetPIDToDefault = false; /* we dont want to do a full reset. */

    /* associate pid channel with bypass keyslot. */
    pPidChannelConf->ucKeySlotType    = BHSM_BYPASS_KEYSLOT_TYPE;
    pPidChannelConf->unKeySlotNum     = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    pPidChannelConf->unKeySlotBType   = BHSM_BYPASS_KEYSLOT_TYPE;
    pPidChannelConf->unKeySlotNumberB = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    #else
    pPidChannelConf->bResetPIDToDefault = true;
    #endif

    if( ( errCode = BHSM_ConfigPidKeyPointerTable( hHsm, pPidChannelConf ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( errCode );
    }

    BDBG_LEAVE(BHSM_ConfigPidChannelToDefaultKeySlot);
    return errCode;
}

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
BERR_Code  BHSM_SetPidChannelBypassKeyslot(
    BHSM_Handle hHsm,
    unsigned pidChannelIndex,
    BHSM_BypassKeySlot_e bypassKeyslot )
{

    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t  status = 0;

    BDBG_ENTER( BHSM_SetPidChannelBypassKeyslot );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pidChannelIndex >= BCMD_TOTAL_PIDCHANNELS )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc ); /* failed create a BSP message  */
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_CONFIG_PIDKEYPOINTERTABLE, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotType, BHSM_BYPASS_KEYSLOT_TYPE );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotNumber, bypassKeyslot );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotTypeB, BHSM_BYPASS_KEYSLOT_TYPE );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotNumberB, bypassKeyslot );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eKeyPointerSel,0 );
    BHSM_BspMsg_Pack32( hMsg, BCMD_KeyPointer_InCmdCfg_ePidChan, pidChannelIndex );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc ); /* error sending message to BSP */
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );

    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", __FUNCTION__, status ));
        goto BHSM_P_DONE_LABEL;
    }

    hHsm->mapPidChannelToKeySlot[pidChannelIndex][BHSM_PidChannelType_ePrimary].keySlotType  = BHSM_BYPASS_KEYSLOT_TYPE;
    hHsm->mapPidChannelToKeySlot[pidChannelIndex][BHSM_PidChannelType_ePrimary].unKeySlotNum = bypassKeyslot;

    BHSM_P_DONE_LABEL:

    if( hMsg )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_SetPidChannelBypassKeyslot );
    return rc;
}

BERR_Code  BHSM_GetPidChannelBypassKeyslot(
    BHSM_Handle hHsm,
    unsigned pidChannelIndex,
    BHSM_BypassKeySlot_e *pBypassKeyslot )
{
    unsigned    keySlotNum;

    BDBG_ENTER( BHSM_GetPidChannelBypassKeyslot );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pBypassKeyslot == NULL )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( pidChannelIndex >= BCMD_TOTAL_PIDCHANNELS || !pBypassKeyslot )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    keySlotNum = hHsm->mapPidChannelToKeySlot[pidChannelIndex][BHSM_PidChannelType_ePrimary].unKeySlotNum;

    *pBypassKeyslot = BHSM_BypassKeySlot_eInvalid;

    if( ( keySlotNum == (unsigned)BHSM_BypassKeySlot_eG2GR ) ||
        ( keySlotNum == (unsigned)BHSM_BypassKeySlot_eGR2R ) )
    {
        *pBypassKeyslot = (BHSM_BypassKeySlot_e)keySlotNum;
    }

    BDBG_LEAVE( BHSM_GetPidChannelBypassKeyslot );

    return BERR_SUCCESS;
}
#endif

BERR_Code BHSM_InvalidateTransportKeySlots( BHSM_Handle hHsm, BHSM_ClientType_e ownerShip )
{
    unsigned int      i, j;
    unsigned int      maxKeySlot;
    BHSM_InvalidateKeyIO_t  invalidateKeyIO;
    BERR_Code    rc = BERR_SUCCESS;

    BSTD_UNUSED( ownerShip ) ;

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    BKNI_Memset( &invalidateKeyIO, 0, sizeof(invalidateKeyIO) );

    /* For each of the key slot type  */
    for( i = 0; i < BCMD_XptSecKeySlot_eTypeMax; i++ ) /* For each ks type */
    {
        maxKeySlot = hHsm->keySlotTypes[i].unKeySlotNum;

        for( j = 0; j < maxKeySlot; j++ ) /* For each ks of type */
        {
            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            invalidateKeyIO.bInvalidateAllEntries = true;
            #endif
            invalidateKeyIO.caKeySlotType         = i;
            invalidateKeyIO.invalidKeyType        = BCMD_InvalidateKey_Flag_eDestKeyOnly;
            invalidateKeyIO.unKeySlotNum          = j;
            invalidateKeyIO.virtualKeyLadderID = BCMD_VKL_KeyRam_eMax;

            rc = BHSM_InvalidateKey(hHsm, &invalidateKeyIO);
            if( rc )
            {
                BDBG_MSG(("Failed key invalidation: Key Type : %d  Key Number  %d", i, j));
                return rc;
            }
        }
    }

    return rc;
}

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)

/* Initialise PID Channels:
 *  -  associate all pidChannels with G2G keyslot.  */
static BERR_Code InitialisePidChannels ( BHSM_Handle hHsm )
{
    BHSM_ConfigPidKeyPointerTableIO_t pidChannelConf;
    BERR_Code       rc = BERR_SUCCESS;
    int             i = 0;

    BDBG_ENTER ( InitialisePidChannels );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    BKNI_Memset ( &pidChannelConf, 0, sizeof ( pidChannelConf ) );
    pidChannelConf.ucKeySlotType = BHSM_BYPASS_KEYSLOT_TYPE;
    pidChannelConf.unKeySlotNum = BHSM_BYPASS_KEYSLOT_NUMBER_g2gr;
    pidChannelConf.pidChannelType = BHSM_PidChannelType_ePrimary;
    pidChannelConf.bResetPIDToDefault = false;

    if(   BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2) &&
          hHsm->firmwareVersion.bseck.major >= BHSM_BSP_FW_VERSION_KEYSLOT_MULTIPLE_PID_CHANNELS )
    {
        pidChannelConf.setMultiplePidChannels = true;
        pidChannelConf.unPidChannel = 0;
        pidChannelConf.unPidChannelEnd = BCMD_TOTAL_PIDCHANNELS - 1;
        if ( ( rc = ConfigPidKeyPointerTable ( hHsm, &pidChannelConf, true /*verbose */  ) ) != BERR_SUCCESS )
        {
            ( void ) BERR_TRACE ( rc );
            goto BHSM_P_DONE_LABEL;
        }

        for ( i = 0; i < BCMD_TOTAL_PIDCHANNELS; i++ )
        {
            hHsm->mapPidChannelToKeySlot[i][BHSM_PidChannelType_ePrimary].keySlotType = pidChannelConf.ucKeySlotType;
            hHsm->mapPidChannelToKeySlot[i][BHSM_PidChannelType_ePrimary].unKeySlotNum = pidChannelConf.unKeySlotNum;
        }
    }
    else
    {
        for ( i = 0; i < BCMD_TOTAL_PIDCHANNELS; i++ )
        {
            pidChannelConf.setMultiplePidChannels = false;
            pidChannelConf.unPidChannel = i;

            if ( ( rc = ConfigPidKeyPointerTable ( hHsm, &pidChannelConf, false /*verbose */  ) ) != BERR_SUCCESS )
            {
                ( void ) BERR_TRACE ( rc );
                goto BHSM_P_DONE_LABEL;
            }

            hHsm->mapPidChannelToKeySlot[i][BHSM_PidChannelType_ePrimary].keySlotType = pidChannelConf.ucKeySlotType;
            hHsm->mapPidChannelToKeySlot[i][BHSM_PidChannelType_ePrimary].unKeySlotNum = pidChannelConf.unKeySlotNum;
        }
    }

  BHSM_P_DONE_LABEL:

    BDBG_LEAVE ( InitialisePidChannels );

    return rc;
}
#endif


/* check if we are running with FullROM or BSECK code */
bool isSecurityInRom( BHSM_Handle hHsm )
{
    BREG_Handle reg = hHsm->regHandle;
    uint32_t regVal;

    regVal = BREG_Read32( reg, BCHP_BSP_GLB_CONTROL_FW_FLAGS );

    if (  ((regVal & 0x07000000) == 0x07000000) ||       /* we are running with BSECK code */
          ((regVal & 0x08000000) == 0x08000000) )        /* we are running with BSECK reload code */
    {
        BDBG_MSG(("Running with BSECK code."));
        return false;

    }

    BDBG_MSG(("Running with FullROM code."));

    return true;
}


/* Map keyslot type and number to keyslot storage. */
BERR_Code BHSM_P_GetKeySlot( BHSM_Handle hHsm, BHSM_P_CAKeySlotInfo_t **ppKeyslot, BCMD_XptSecKeySlot_e keySlotType, unsigned keySlotNum )
{
    unsigned char keySlotStartOffset = 0 ;
    BHSM_P_CAKeySlotInfo_t *pKeyslot =  NULL;

    BDBG_ENTER( BHSM_P_GetKeySlot );

    if( keySlotType >= BCMD_XptSecKeySlot_eTypeMax )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( keySlotNum > hHsm->keySlotTypes[keySlotType].unKeySlotNum )
    {
        BDBG_ERR(("Invalid KeySlot#[%d] Type[%d]", keySlotNum, keySlotType ));
        return BHSM_STATUS_INPUT_PARM_ERR;
    }

    keySlotStartOffset  = hHsm->keySlotTypes[keySlotType].ucKeySlotStartOffset;

    if( (keySlotStartOffset + keySlotNum) > BHSM_MAX_KEYLSOTS )
    {
        BDBG_ERR(("Invalid KeySlot#[%d] Type[%d]", keySlotNum, keySlotType ));
        return BHSM_STATUS_INPUT_PARM_ERR;
    }

    pKeyslot = hHsm->pKeySlot[keySlotStartOffset + keySlotNum];

    if( pKeyslot == NULL )
    {
        /* Keyslot not allocated. . */
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    *ppKeyslot = pKeyslot;

    BDBG_LEAVE( BHSM_P_GetKeySlot );
    return BERR_SUCCESS;
}

#if BHSM_SUPPORT_KEYSLOT_OWNERSHIP
/* returns true of keyslot type is reserved for Zzyzz CA Provider */
static bool isKeySlotTypeZzyzx( BCMD_XptSecKeySlot_e keySlotType )
{
    if(   keySlotType == BCMD_XptSecKeySlot_eType1
       || keySlotType == BCMD_XptSecKeySlot_eType4
      #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
       || keySlotType == BCMD_XptSecKeySlot_eType5
      #endif
          )
    {
        return true;
    }

    return false;
}
#endif


#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)

static BERR_Code mapSlotType2StatusQueryOffset ( BCMD_XptSecKeySlot_e keySlotType,
                                                 BCMD_KeySlotStatusQuery_OutCmd_e * pOffset )
{
    BDBG_ENTER ( mapSlotType2StatusQueryOffset );

    if ( pOffset == NULL )
    {
        return BERR_TRACE ( BERR_INVALID_PARAMETER );
    }

    switch ( keySlotType )
    {
        case BCMD_XptSecKeySlot_eType0: { *pOffset = BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType0; break; }
        case BCMD_XptSecKeySlot_eType1: { *pOffset = BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType1; break; }
        case BCMD_XptSecKeySlot_eType2: { *pOffset = BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType2; break; }
        case BCMD_XptSecKeySlot_eType3: { *pOffset = BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType3; break; }
        case BCMD_XptSecKeySlot_eType4: { *pOffset = BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType4; break; }
        case BCMD_XptSecKeySlot_eType5: { *pOffset = BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType5; break; }
        default:
        {
            return BERR_TRACE ( BHSM_STATUS_FAILED );
        }
    }

    BDBG_LEAVE ( mapSlotType2StatusQueryOffset );
    return BERR_SUCCESS;
}

BERR_Code BHSM_QueryKeySlotsStatus ( BHSM_Handle hHsm, BHSM_KeySlotsStatus_t * pKeySlotsStatus  /*output, the types and keyslot numbers for each ownership. */
     )
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t         status = 0;
    uint16_t        responseLength = 0;
    uint8_t         slotNumOfType = 0;
	uint32_t        slotNumOffset = 0;
    uint8_t         owner = 0;
    BCMD_KeySlotStatusQuery_OutCmd_e keySlotQueryOffset;
    BCMD_XptSecKeySlot_e keySlotType;
    unsigned int             keySlotNum;

    BDBG_ENTER ( BHSM_QueryKeySlotsStatus );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if ( pKeySlotsStatus == NULL )
    {
        return BERR_TRACE ( BHSM_STATUS_FAILED );
    }

    BKNI_Memset ( pKeySlotsStatus, 0, sizeof ( BHSM_KeySlotsStatus_t ) );

    if ( ( rc = BHSM_BspMsg_Create ( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE ( rc );      /* failed create a BSP message  */
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader ( &header );
    BHSM_BspMsg_Header ( hMsg, BCMD_cmdType_eKEY_SLOT_STATUS_QUERY, &header );

    if ( ( rc = BHSM_BspMsg_SubmitCommand ( hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE ( rc );      /* error sending message to BSP */
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 ( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eStatus, &status );

    if ( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR ( ( "%s BSP status[0x%02X] error", __FUNCTION__, status ) );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get16 ( hMsg, BCMD_CommonBufferFields_eParamLen, &responseLength );

    #define   BHSM_MAX_SLOTS_STATUS_BUFFER_BYTE_LEN (236)

    /* Minus status and slot numbers for six types' length */
    if ( responseLength - 7 > BHSM_MAX_SLOTS_STATUS_BUFFER_BYTE_LEN )
    {
        rc = BERR_TRACE ( BHSM_STATUS_BSP_ERROR );
        goto BHSM_P_DONE_LABEL;
    }
    /* The key slot status information is expected as the following byte order. */

    /* |type0 slot0, type0 slot1, type0 slot2 ... | */
    /* |type1 slot0, type1 slot1, type1 slot2 ... | */
    /* | ...                                      | */
    /* |type5 slot0, type5 slot1, type5 slot2 ... | */

    for ( keySlotType = BCMD_XptSecKeySlot_eType0; keySlotType <= BHSM_KeySlotStatusQuery_eTypeMax; keySlotType++ )
    {
        rc = mapSlotType2StatusQueryOffset ( keySlotType, &keySlotQueryOffset );
        if ( rc != BERR_SUCCESS )
        {
            BERR_TRACE ( rc );
            goto BHSM_P_DONE_LABEL;
        }

        /* Query the number of key slot of keySlotType. */
        BHSM_BspMsg_Get8 ( hMsg, keySlotQueryOffset, &slotNumOfType );

        for ( keySlotNum = 0; keySlotNum <= slotNumOfType; keySlotNum++ )
        {
            slotNumOffset = pKeySlotsStatus->numKeyslots + keySlotNum;

            /* Set the key slot type information for the output. */
            pKeySlotsStatus->slot[slotNumOffset].type = keySlotType;
            pKeySlotsStatus->slot[slotNumOffset].number = slotNumOfType;

            /* Query the ownership information of keySlotType. */
            BHSM_BspMsg_Get8 ( hMsg,
                               BCMD_KeySlotStatusQuery_OutCmd_eKeySlotOwnership + slotNumOffset,
                               &owner);

            pKeySlotsStatus->slot[slotNumOffset].owner = owner;
        }

        pKeySlotsStatus->numKeyslots += slotNumOfType;
    }

    if ( pKeySlotsStatus->numKeyslots >= BHSM_MAX_KEYLSOTS )
    {
        rc = BERR_TRACE ( BHSM_STATUS_BSP_ERROR );
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    if ( hMsg )
    {
        ( void ) BHSM_BspMsg_Destroy ( hMsg );
    }

    BDBG_LEAVE ( BHSM_QueryKeySlotsStatus );
    return rc;
}

#endif /* #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2) */


/* Load the HSM stucture with the BFW version.
    - Call a random BSP command (SESSION_GENERATE_ROUTE_KEY, Query VKL, used as its expect to require minimum impact on BSP )
    - Parse the BSP response to determine the version parameters
    - print version to console.
     */
BERR_Code  loadBspVersion( BHSM_Handle hHsm )
{
    BERR_Code              rc = BERR_SUCCESS;
    BHSM_BspMsg_h          hMsg = NULL;
    BHSM_BspMsgHeader_t    header;
    uint8_t                status  = 0;
    uint8_t                byte  = 0;
    uint32_t               version = 0;
    uint8_t                versionScheme = 0;  /* 0 for old, 1 for new */
    uint8_t                bfwEpoch = 0;
    char                   message[128] = "";
    int                    bytesLeft = 0;

    BDBG_ENTER( loadBspVersion );

	/* hHsm is valid, as BHSM_P_Handle object is set after this function is called. */

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY, &header ); /* need to use some command, selecting GENERATE_ROUTE_KEY, query VKL. */

    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLAssociationQuery, BCMD_VKLAssociationQueryFlag_eQuery );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLadderType, BCMD_KeyLadderType_e3DESABA );
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eNoKeyGenFlag, BCMD_KeyGenFlag_eNoGen );
    #endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eVKLID, BCMD_VKL0 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_GenKey_InCmd_eKeyLayer, BCMD_KeyRamBuf_eKey3 );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) != BERR_SUCCESS ) )
    {
        BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", __FUNCTION__, status ));
        goto BHSM_P_DONE_LABEL;
    }

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eVersionScheme, &byte );
   #endif
    versionScheme = byte & 0x0F;    /* 4 bits */
    bfwEpoch      = byte >> 4;      /* 4 bits */
    /* Deteremine the Firmware versions  */
    BHSM_BspMsg_Get32( hMsg, BCMD_CommonBufferFields_eVerNum, &version );


    /* parse version from BSP response message */
    BKNI_Memset( &hHsm->firmwareVersion, 0, sizeof(BHSM_FirmwareVersion) );
    if( versionScheme == 0 )
    {
        hHsm->firmwareVersion.platform.major      =  BHSM_ZEUS_VER_MAJOR;
        hHsm->firmwareVersion.platform.minor      =  BHSM_ZEUS_VER_MINOR;

        /*pre Zeus4.1*/
        hHsm->firmwareVersion.bseck.major         =  ((version >> 12) & 0xF ); /* 4 bits */
        hHsm->firmwareVersion.bseck.minor         =  ((version >>  6) & 0x3F); /* 6 bits */
        hHsm->firmwareVersion.customerModes       =  ((version >>  0) & 0x3F); /* 6 bits */
    }
    else
    {
        hHsm->firmwareVersion.platform.major      =  ((version >> 28) & 0xF);  /* 4 bits */
        hHsm->firmwareVersion.platform.minor      =  ((version >> 24) & 0xF);  /* 4 bits */
        hHsm->firmwareVersion.platform.subMinor   =  ((version >> 22) & 0x3);  /* 2 bits */
        hHsm->firmwareVersion.bseck.major         =  ((version >> 18) & 0xF);  /* 4 bits */
        hHsm->firmwareVersion.bseck.minor         =  ((version >> 12) & 0x3F); /* 6 bits */
        hHsm->firmwareVersion.bseck.subMinor      =  ((version >>  8) & 0xF);  /* 4 bits */
                                                             /* 2 reserved  bits */
        hHsm->firmwareVersion.verified            =  ((version >>  4) & 0x3) ? false : true;  /* 2 bits, true of Zero  */
        hHsm->firmwareVersion.customerModes       =  ((version >>  0) & 0xF);  /* 4 bits */

        if( versionScheme == 2 )
        {
            hHsm->firmwareVersion.bfwEpoch.valid = true;
            hHsm->firmwareVersion.bfwEpoch.value = bfwEpoch;
        }
    }

    /* print version information to screen */
    bytesLeft = sizeof( message );
    bytesLeft -= BKNI_Snprintf( &message[sizeof(message)-bytesLeft], bytesLeft, "BSP Platform[%d.%d.%d]", hHsm->firmwareVersion.platform.major
                                                                                                        , hHsm->firmwareVersion.platform.minor
                                                                                                        , hHsm->firmwareVersion.platform.subMinor );

    bytesLeft -= BKNI_Snprintf( &message[sizeof(message)-bytesLeft], bytesLeft, " BSECK[%d.%d.%d]", hHsm->firmwareVersion.bseck.major
                                                                                                  , hHsm->firmwareVersion.bseck.minor
                                                                                                  , hHsm->firmwareVersion.bseck.subMinor );
    if( versionScheme > 0 )
    {
        bytesLeft -= BKNI_Snprintf( &message[sizeof(message)-bytesLeft], bytesLeft, " verified[%s]",  hHsm->firmwareVersion.verified ? "yes" : "no" );
    }

    bytesLeft -= BKNI_Snprintf( &message[sizeof(message)-bytesLeft], bytesLeft, " modes[0x%02X]", hHsm->firmwareVersion.customerModes );

    if( hHsm->firmwareVersion.bfwEpoch.valid )
    {
        bytesLeft -= BKNI_Snprintf( &message[sizeof(message)-bytesLeft], bytesLeft, " BFW Epoch[%d]", hHsm->firmwareVersion.bfwEpoch.value );
    }

    #if BHSM_SUPPORT_DEBUG_READ_OTP_TYPE
    {
        char otpType[3] = "--";
        BHSM_DEBUG_GetChipsetOtpType( hHsm, &otpType[0], &otpType[1] );
        bytesLeft -= BKNI_Snprintf( &message[sizeof(message)-bytesLeft], bytesLeft, " otpType[%s]", otpType );
    }
    #endif

    /* WARNING used so as to make information available to system integrators. */
    BDBG_WRN(( "%s", message ));

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( loadBspVersion );
    return rc;
}


/* Returns true if current BFW is greater than or equal to specified value */
bool isBfwVersion_GreaterOrEqual ( BHSM_Handle hHsm, unsigned major, unsigned minor, unsigned subMinor )
{
    if ( major < hHsm->firmwareVersion.bseck.major )
    {
        return true;
    }
    else if ( major == hHsm->firmwareVersion.bseck.major )
    {
        if ( minor < hHsm->firmwareVersion.bseck.minor )
        {
            return true;
        }
        else if ( minor == hHsm->firmwareVersion.bseck.minor )
        {
            return ( subMinor <= hHsm->firmwareVersion.bseck.subMinor );
        }
    }

    return false;
}
