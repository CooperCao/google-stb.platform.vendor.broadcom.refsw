/***************************************************************************
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
 **************************************************************************/

#include "bmmt_security.h"
#include "inttypes.h"

BDBG_MODULE(bmmt_security);
#define XPT_TS_PACKET_SIZE (188)

#undef MIN
#define MIN(A,B) ((A)<(B)?(A):(B))

#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) {             \
    int x_offset;                                                       \
    printf("[%s][%u]", description_txt, (unsigned int) in_size );       \
    for( x_offset = 0; x_offset < (int)(in_size); x_offset++ )          \
    {                                                                   \
        if( x_offset%16 == 0 ) { printf("\n"); }                        \
        printf("%02X ", in_ptr[x_offset] );                             \
    }                                                                   \
    printf("\n");                                                       \
}
typedef struct bmmt_security
{
   BDBG_OBJECT(bmmt_security)
   NEXUS_KeySlotHandle keyslotHandle;
   NEXUS_KeySlotIv slotIv;
   NEXUS_KeySlotKey slotKey;
   NEXUS_DmaHandle dma;
   NEXUS_DmaJobHandle dmaJob;
   BKNI_EventHandle dmaEvent;
   NEXUS_KeySlotExternalKeyData ext_keyslot_data;
   uint8_t *iv;
   uint8_t *key;
   uint8_t *btp_src;
   uint8_t *btp_dest;
}bmmt_security;

#if BMMT_EXTERNAL_KEY_IV == 1
typedef struct bmmt_security_external_key_data
{
    unsigned  slot_index;
    struct
    {
        bool valid;
        unsigned offset;
        unsigned size;
        uint8_t *data;
    } key, iv;
} bmmt_security_external_key_data_t;
#endif

static void bmmt_security_p_dma_complete_cb( void *pParam, int iParam )
{
    BSTD_UNUSED( iParam );
    BKNI_SetEvent( pParam );
    return;
}

#if BMMT_EXTERNAL_KEY_IV == 1
void bmmt_security_p_compile_btp( uint8_t * btp, bmmt_security_external_key_data_t * btp_data )
{
    unsigned char *p = btp;
    unsigned      len = 0;
    unsigned char template_btp[] = {
        /* ( 0) */ 0x47,
        /* ( 1) */ 0x00,
        /* ( 2) */ 0x21,
        /* ( 3) */ 0x20,
        /* ( 4) */ 0xb7,
        /* ( 5) */ 0x82,
        /* ( 6) */ 0x45,
        /* ( 7) */ 0x00,
        /* ( 8) */ 0x42,
        /*'B' */
        /* ( 9) */ 0x52,
        /*'R' */
        /* (10) */ 0x43,
        /*'C' */
        /* (11) */ 0x4d,
        /*'M' */
        /* (12) */ 0x00,
        /* (13) */ 0x00,
        /* (14) */ 0x00,
        /* (15) */ 0x1a
            /* security BTP */
    };
    BDBG_ASSERT( btp );
    BDBG_ASSERT( btp_data );
    BDBG_ASSERT( sizeof( template_btp ) <= XPT_TS_PACKET_SIZE );
    BKNI_Memset( btp, 0, XPT_TS_PACKET_SIZE );
    BKNI_Memcpy( btp, template_btp, sizeof( template_btp ) );
    btp[18] = ( btp_data->slot_index >> 8 ) & 0xFF;
    btp[19] = btp_data->slot_index & 0xFF;
    if( btp_data->key.valid ) {
        p = &btp[20];
        p += ( btp_data->key.offset * 16 );

        len = btp_data->key.size;
        btp_data->key.data += ( len - 8 );
        while( len ) {
            BKNI_Memcpy( p, btp_data->key.data, MIN( len, 8 ) );
            BKNI_Memset( ( p + 8 ), 0xFF, MIN( len, 8 ) );
            p += 16;
            len -= MIN( len, 8 );
            btp_data->key.data -= MIN( len, 8 );
        }
    }
    if( btp_data->iv.valid ) {

        p = &btp[20];
        p += ( btp_data->iv.offset * 16 );
        len = btp_data->iv.size;
        btp_data->iv.data += ( len - 8 );
        while( len ) {
            BKNI_Memcpy( p, btp_data->iv.data, MIN( len, 8 ) );    /* set IV    */
            BKNI_Memset( p + 8, 0xFF, MIN( len, 8 ) );  /* set Mask */
            p += 16;
            len -= MIN( len, 8 );
            btp_data->iv.data -= MIN( len, 8 );
        }
    }
    return;
}
#endif
int bmmt_security_dma_transfer( bmmt_security_t mmt_security, NEXUS_DmaJobBlockSettings *block_settings, int num_blocks )
{
    NEXUS_Error   rc;
    NEXUS_DmaJobStatus jobStatus;
    #if BMMT_EXTERNAL_KEY_IV == 1
    bmmt_security_external_key_data_t ext_key_data;
    #endif
    int i=0;

    #if BMMT_EXTERNAL_KEY_IV == 1
    BKNI_Memset( &ext_key_data, 0, sizeof( ext_key_data ) );
    ext_key_data.slot_index = mmt_security->ext_keyslot_data.slotIndex;
    ext_key_data.key.valid = true;
    ext_key_data.key.offset = mmt_security->ext_keyslot_data.key.offset;
    ext_key_data.key.size = BMMT_MAX_AES_CTR_KEY_SIZE;
    ext_key_data.key.data = mmt_security->key;
    ext_key_data.iv.valid = true;
    ext_key_data.iv.offset = mmt_security->ext_keyslot_data.iv.offset;
    ext_key_data.iv.size = BMMT_MAX_IV_SIZE;
    ext_key_data.iv.data = mmt_security->iv;
    BKNI_Memset( mmt_security->btp_src, 0, XPT_TS_PACKET_SIZE );
    BKNI_Memset( mmt_security->btp_dest, 0, XPT_TS_PACKET_SIZE );
    bmmt_security_p_compile_btp( mmt_security->btp_src, &ext_key_data);
    NEXUS_DmaJob_GetDefaultBlockSettings( &block_settings[0] );
    block_settings[0].resetCrypto = true;
    block_settings[0].cached = true;
    block_settings[0].scatterGatherCryptoStart = true;
    block_settings[0].scatterGatherCryptoEnd = true;
    block_settings[0].securityBtp = true;
    block_settings[0].pSrcAddr = mmt_security->btp_src;
    block_settings[0].pDestAddr =  mmt_security->btp_dest;
    block_settings[0].blockSize = XPT_TS_PACKET_SIZE;
    for (i=1;i<num_blocks;i++)
    #else
    for (i=0;i<num_blocks;i++)
    #endif
    {
       block_settings[i].resetCrypto = true;
       block_settings[i].cached = true;
       #if BMMT_EXTERNAL_KEY_IV == 1
       block_settings[i].scatterGatherCryptoStart = (i==1)?true:false;
       #else
       block_settings[i].scatterGatherCryptoStart = (i==0)?true:false;
       #endif
       block_settings[i].scatterGatherCryptoEnd = (i==num_blocks-1)?true:false;
       block_settings[i].securityBtp = false;
    }
    rc = NEXUS_DmaJob_ProcessBlocks( mmt_security->dmaJob, block_settings, num_blocks );
    if( rc == NEXUS_DMA_QUEUED )
    {
        BKNI_WaitForEvent( mmt_security->dmaEvent, 100);
        NEXUS_DmaJob_GetStatus( mmt_security->dmaJob, &jobStatus );
        BDBG_ASSERT( jobStatus.currentState == NEXUS_DmaJobState_eComplete );
        rc = NEXUS_SUCCESS;
    }
    return rc;
}

bmmt_security_t bmmt_security_init(void)
{
    bmmt_security_t mmt_security;
    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotBlockEntry entry = NEXUS_KeySlotBlockEntry_eMax;
    NEXUS_Error   rc = NEXUS_UNKNOWN;
    NEXUS_DmaJobSettings jobSettings;

    mmt_security = BKNI_Malloc(sizeof(*mmt_security));
    BDBG_ASSERT((mmt_security));

    BKNI_Memset(mmt_security,0,sizeof(*mmt_security));
    /* Allocate a key slot. */
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerEntry;
    keyslotAllocSettings.useWithDma = true;

    mmt_security->keyslotHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );
    BDBG_ASSERT((mmt_security->keyslotHandle));

    /* Configure the keyslot. */
    NEXUS_KeySlot_GetSettings( mmt_security->keyslotHandle, &keyslotSettings );

    rc = NEXUS_KeySlot_SetSettings( mmt_security->keyslotHandle, &keyslotSettings );
    BDBG_ASSERT((rc == 0));

    /* Configure a key entry to decrypt. */
    entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    NEXUS_KeySlot_GetEntrySettings( mmt_security->keyslotHandle, entry, &keyslotEntrySettings );
    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eCounter;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.solitaryMode = NEXUS_KeySlotTerminationSolitaryMode_eClear; /*= NEXUS_KeySlotTerminationSolitaryMode_eIv1*/;
    keyslotEntrySettings.counterMode = NEXUS_CounterMode_e1;
    keyslotEntrySettings.counterSize = 128;
    #if BMMT_EXTERNAL_KEY_IV == 1
    keyslotEntrySettings.external.iv = true;
    keyslotEntrySettings.external.key = true;
    #else
    keyslotEntrySettings.external.iv = false;
    keyslotEntrySettings.external.key = false;
    #endif
    keyslotEntrySettings.rPipeEnable = true;
    keyslotEntrySettings.gPipeEnable = true;
    rc = NEXUS_KeySlot_SetEntrySettings( mmt_security->keyslotHandle, entry, &keyslotEntrySettings );
    BDBG_ASSERT((rc == 0));

    #if BMMT_EXTERNAL_KEY_IV == 1
    rc = NEXUS_KeySlot_GetEntryExternalKeySettings( mmt_security->keyslotHandle, entry, &mmt_security->ext_keyslot_data );
    #endif
    mmt_security->dma = NEXUS_Dma_Open( NEXUS_ANY_ID, NULL );
    BDBG_ASSERT(( mmt_security->dma ));
    BKNI_CreateEvent( &mmt_security->dmaEvent );

    NEXUS_DmaJob_GetDefaultSettings( &jobSettings );
    jobSettings.numBlocks = BMMT_MAX_DMA_BLOCKS;
    jobSettings.keySlot = mmt_security->keyslotHandle;
    jobSettings.dataFormat = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = bmmt_security_p_dma_complete_cb;
    jobSettings.completionCallback.context = mmt_security->dmaEvent;
    mmt_security->dmaJob = NEXUS_DmaJob_Create( mmt_security->dma, &jobSettings );
    BDBG_ASSERT((mmt_security->dmaJob));

    #if BMMT_EXTERNAL_KEY_IV == 1
    NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &mmt_security->btp_src );
    BDBG_ASSERT((mmt_security->btp_src));
    NEXUS_Memory_Allocate( XPT_TS_PACKET_SIZE, NULL, ( void ** ) &mmt_security->btp_dest );
    BDBG_ASSERT((mmt_security->btp_dest));
    BKNI_Memset( mmt_security->btp_src, 0, XPT_TS_PACKET_SIZE );
    BKNI_Memset( mmt_security->btp_dest, 0, XPT_TS_PACKET_SIZE );
    #endif
    return mmt_security;
}

void bmmt_security_uninit( bmmt_security_t mmt_security)
{
    #if BMMT_EXTERNAL_KEY_IV == 1
    NEXUS_Memory_Free(mmt_security->btp_src);
    NEXUS_Memory_Free(mmt_security->btp_dest);
    #endif
    NEXUS_KeySlot_Free(mmt_security->keyslotHandle);
    NEXUS_DmaJob_Destroy( mmt_security->dmaJob );
    NEXUS_Dma_Close( mmt_security->dma );
    BKNI_DestroyEvent( mmt_security->dmaEvent );
    BKNI_Free(mmt_security);
    return;
}


int bmmt_security_load_key(bmmt_security_t mmt_security, uint8_t *key)
{
    #if BMMT_EXTERNAL_KEY_IV == 1
    mmt_security->key = key;
    #else
    NEXUS_Error   rc = NEXUS_UNKNOWN;
    NEXUS_KeySlotBlockEntry entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    mmt_security->slotKey.size = BMMT_MAX_AES_CTR_KEY_SIZE;
    BKNI_Memcpy( mmt_security->slotKey.key, key, BMMT_MAX_AES_CTR_KEY_SIZE );
    /*DEBUG_PRINT_ARRAY( "key", BMMT_MAX_AES_CTR_KEY_SIZE, key );*/
    rc = NEXUS_KeySlot_SetEntryKey( mmt_security->keyslotHandle, entry, &mmt_security->slotKey );
    BDBG_ASSERT((rc == 0));
    #endif
    return 0;
}

int bmmt_security_load_IV(bmmt_security_t mmt_security, uint8_t *iv)
{
    #if BMMT_EXTERNAL_KEY_IV == 1
    mmt_security->iv = iv;
    #else
    NEXUS_Error   rc = NEXUS_UNKNOWN;
    NEXUS_KeySlotBlockEntry entry = NEXUS_KeySlotBlockEntry_eCpdClear;
    mmt_security->slotIv.size = BMMT_MAX_IV_SIZE;
    BKNI_Memcpy( mmt_security->slotIv.iv, iv, BMMT_MAX_IV_SIZE );
    /*DEBUG_PRINT_ARRAY( "IV", BMMT_MAX_IV_SIZE, iv );*/
    rc = NEXUS_KeySlot_SetEntryIv( mmt_security->keyslotHandle, entry, &mmt_security->slotIv, NULL );
    BDBG_ASSERT((rc == 0));
    #endif
    return 0;
}
