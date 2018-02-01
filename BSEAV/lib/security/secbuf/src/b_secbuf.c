/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 **************************************************************************/
/*! \file b_secbuf.c
 *  \brief define b_secbuf functions.
 */
/*#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"
*/

#include "nexus_platform.h"
#include "nexus_security.h"
#include "nexus_dma.h"
/* #include "nxclient.h" */

#include "b_secbuf.h"
#include "blst_list.h"
#include"blst_aa_tree.h"
#include "pthread.h"

#define B_SECBUF_MAGIC 0x6272636f   /* 'brcm'  */

BDBG_MODULE(b_secbuf);

/*
 * set to 0 for now as ComcastSEC_DecryptDataOpaqueWithOffset()
 * does not support SAGE buffer yet.
 */
/* #define USE_SECURE_MEM 1 */
static int use_secure_mem = 1;
/**
Summary:
Opaque handle for secbuf
**/
typedef struct B_Secbuf * B_Secbuf_T;


/**
Summary:
Settings for secbuf
**/
typedef struct _B_Secbuf_Settings {
    void * generic_heap; /* [in/out] generic heap, and used to allocate DMA-capable buffer,
			  *  this heap is RW, buffer allocated in this heap will be used in GetBuffer()
			  *  call and returned to caller. Caller fills in the buffer, and call WriteComplete()
			  *  to DMA the buffer to playpump heap. If NULL, secbuf will assign a heap internally.
			  */
    void * secure_heap; /* [in/out] secure heap, and used to allocate DMA-capable buffer, non-cpu access */

} B_Secbuf_Settings;

#define DEFAULT_NUM_DMA_BLOCKS 32
typedef struct B_Secbuf_Desc_Priv {
    uint8_t * addr_cache; /* */
    NEXUS_DmaJobBlockSettings * dma_blocks;
    int num_dma_blocks;
    int cur_dma_blocks;
};

typedef struct B_Secbuf_Desc_Priv * B_Secbuf_Desc_Priv_T;



struct _chunk_info {
    uint32_t clear_size;
    uint32_t encrypted_size;
};


struct B_Secbuf_Desc
{
    BLST_AA_TREE_ENTRY(B_Secbuf_Desc) node;
    unsigned int magic;
    void (* free_callback)(void * param); /* free function */
    unsigned char * addr; /* address of the buffer*/
    unsigned int len;  /* size of the buffer */
    B_Secbuf_Type type; /* buffer type */
    NEXUS_MemoryBlockTokenHandle token; /* nexus token handle to share device memory */
    void * priv; /* used internally*/
    uint32_t  offset; /* */
};
typedef struct B_Secbuf_Desc * B_Secbuf_Desc_T;

BLST_AA_TREE_HEAD(_B_Secbuf_List_Head, B_Secbuf_Desc);
static int B_Secbuf_Desc_Compare(const struct B_Secbuf_Desc * node, void * addr)
{
    if(addr > node->addr) {
        return 1;
    } else if(addr == node->addr ) {
        return 0;
    } else {
        return -1;
    }
}

BLST_AA_TREE_GENERATE_FIND(_B_Secbuf_List_Head , B_Secbuf_Desc_T, B_Secbuf_Desc, node, B_Secbuf_Desc_Compare)
BLST_AA_TREE_GENERATE_INSERT(_B_Secbuf_List_Head, B_Secbuf_Desc_T, B_Secbuf_Desc, node, B_Secbuf_Desc_Compare)
BLST_AA_TREE_GENERATE_REMOVE(_B_Secbuf_List_Head, B_Secbuf_Desc, node)


/*--------------------------------------function prototypes-------------------------------------------*/


/*! \brief Open Secbuf instance.
 no B_Secbuf_Close(), resource will be released when process exists
 */
B_Error B_Secbuf_Open(void);


/*
Summary:
Throw away all data currently in playback fifo.

Description:
*/


#if 0
#undef BDBG_MSG
#define BDBG_MSG BDBG_LOG
#endif


#define B_SECBUF_DEFAULT_BUF_SIZE (256*1024)
struct B_Secbuf {
	unsigned int magic;
    NEXUS_DmaHandle dmaHandle;
    NEXUS_DmaJobHandle dmaJob;
    BKNI_MutexHandle dma_mutex;

	B_Secbuf_Settings settings;

    unsigned char * dma_buf; /* dma buffer allocated on generic heap for copying data to secure buf*/
    unsigned int dma_buf_len;

    BKNI_MutexHandle desc_list_mutex;

    struct _B_Secbuf_List_Head desc_list;
    uint32_t bytes_alloc;
    uint32_t bytes_alloc_max;
    uint32_t bytes_shared;
    uint32_t bytes_shared_max;
    uint32_t bytes_packet_max;

};

void B_Secbuf_Free_Callback( void * arg);
static B_Error B_Secbuf_p_FindHeap(void **heap, unsigned memType, unsigned heapType);
static B_Error B_Secbuf_P_CopyData(B_Secbuf_Desc_T desc, unsigned char * pDataOut, unsigned char * pDataIn, size_t len, int flush);
static B_Error B_Secbuf_P_DoDma(B_Secbuf_Desc_T desc );
static uint8_t * B_Secbuf_P_Memory_Allocate(B_Secbuf_Desc_T desc, uint32_t size, B_Secbuf_Type memoryType);

static  B_Secbuf_T g_secbuf = NULL;
#define EXTRA_LARGE_PKT 0x400000 /* Unlikely large packet for streaming */
static uint32_t g_max_pkt = EXTRA_LARGE_PKT; /* detecting large packet */

#define SECBUF_OPEN_STATE_NOT_INIT       0 /* not initialized */
#define SECBUF_OPEN_STATE_IN_PROGRESS    1 /* in progress */
#define SECBUF_OPEN_STATE_DONE           2 /* init done */

static  uint32_t g_secbuf_open_state = SECBUF_OPEN_STATE_NOT_INIT;
static pthread_mutex_t g_open_state_mutex = PTHREAD_MUTEX_INITIALIZER;


static B_Secbuf_Desc_T find_desc( void * buffer) {
    struct B_Secbuf_Desc * desc = NULL;
    desc = BLST_AA_TREE_FIND(_B_Secbuf_List_Head, &g_secbuf->desc_list, buffer);
    return desc;
}

/*! \brief Initialize Secbuf module.
 */
B_Error B_Secbuf_Open(void)
{
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_DmaJobSettings dmaJobSettings;
	unsigned int retValue = B_ERROR_UNKNOWN;
    B_Secbuf_T s_secbuf = NULL;

    int i = 0;

    pthread_mutex_lock(&g_open_state_mutex);
    if (SECBUF_OPEN_STATE_NOT_INIT == g_secbuf_open_state)
    {
        g_secbuf_open_state = SECBUF_OPEN_STATE_IN_PROGRESS;
    } else if (SECBUF_OPEN_STATE_IN_PROGRESS == g_secbuf_open_state) {
        /* someone else is calling this function, wait for up to 1 second */
        int ms = 1000;
        BDBG_LOG(("%s in progress ", __FUNCTION__));
        while (ms > 0){
            BKNI_Sleep(50);
            if (SECBUF_OPEN_STATE_DONE == g_secbuf_open_state) break;
            ms -= 50;
        };
        if (SECBUF_OPEN_STATE_DONE == g_secbuf_open_state) {
            retValue = B_ERROR_SUCCESS;
            goto out;
        } else {
            BDBG_ERR(("%s(%d): waited 1s ", __FUNCTION__, __LINE__));
            goto out;
        }

    } else {
        if (SECBUF_OPEN_STATE_DONE == g_secbuf_open_state) {
            retValue = B_ERROR_SUCCESS;
        }
        goto out;
    }

    s_secbuf = BKNI_Malloc(sizeof(struct B_Secbuf));
    if (!s_secbuf) {
        BDBG_ERR(("%s(%d): mem alloc error, type %u", __FUNCTION__, __LINE__));
        retValue = B_ERROR_OUT_OF_MEMORY;
        goto out;
    }
    BKNI_Memset(s_secbuf, 0, sizeof(struct B_Secbuf));

    BLST_AA_TREE_INIT(_B_Secbuf_List_Head, &s_secbuf->desc_list);
    /* create dma handle */
    s_secbuf->dmaHandle = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);
    NEXUS_DmaJob_GetDefaultSettings(&dmaJobSettings);
    dmaJobSettings.completionCallback.callback = NULL;
    if (use_secure_mem)
        dmaJobSettings.bypassKeySlot = NEXUS_BypassKeySlot_eGR2R;
    else
        dmaJobSettings.bypassKeySlot = NEXUS_BypassKeySlot_eG2GR;

    dmaJobSettings.numBlocks =128;
    s_secbuf->dmaJob= NEXUS_DmaJob_Create(s_secbuf->dmaHandle,&dmaJobSettings);
    if (!s_secbuf->dmaJob) {
        BDBG_ERR(("%s(%d) create dmaJob failed", __FUNCTION__, __LINE__));
        retValue == B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    /* search and assign heap */
    retValue = B_Secbuf_p_FindHeap(&s_secbuf->settings.generic_heap, NEXUS_MemoryType_eFull, NEXUS_HEAP_TYPE_MAIN);
    if (retValue != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s(%d) find heap error", __FUNCTION__, __LINE__));
        goto out;
    }
    retValue = B_Secbuf_p_FindHeap(&s_secbuf->settings.secure_heap, NEXUS_MemoryType_eSecure, NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION);
    if (retValue != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s(%d) find heap error", __FUNCTION__, __LINE__));
        goto out;
    }
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = s_secbuf->settings.generic_heap;
    NEXUS_Memory_Allocate(B_SECBUF_DEFAULT_BUF_SIZE, &memSettings, (void *)&s_secbuf->dma_buf);
    if (!s_secbuf->dma_buf) {
        BDBG_ERR(("%s(%d): mem alloc error, type %u", __FUNCTION__, __LINE__));
        retValue = B_ERROR_OUT_OF_MEMORY;
        goto out;
    }
    s_secbuf->dma_buf_len = B_SECBUF_DEFAULT_BUF_SIZE;
    s_secbuf->bytes_alloc += s_secbuf->dma_buf_len;
    if (BKNI_CreateMutex(&s_secbuf->dma_mutex) != 0) {
        retValue = B_ERROR_OS_ERROR;
        goto out;
    }

    if (BKNI_CreateMutex(&s_secbuf->desc_list_mutex) != 0)  {
        retValue = B_ERROR_OS_ERROR;
        goto out;
    }

    g_secbuf = s_secbuf;
    g_secbuf_open_state = SECBUF_OPEN_STATE_DONE;
    retValue = B_ERROR_SUCCESS;
 out:
    if (retValue != B_ERROR_SUCCESS) {
        g_secbuf_open_state = SECBUF_OPEN_STATE_NOT_INIT;
    }
    pthread_mutex_unlock(&g_open_state_mutex);
    return retValue;

}


static B_Error B_Secbuf_p_FindHeap(void **heap, unsigned memType, unsigned heapType)
{
	unsigned int retValue = B_ERROR_SUCCESS;
    NEXUS_ClientConfiguration clientConfig;
    int i;

    *heap = NULL;
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
	NEXUS_MemoryStatus s;
	if (!clientConfig.heap[i] || NEXUS_Heap_GetStatus(clientConfig.heap[i], &s)) continue;
	if (s.memoryType == memType && s.heapType == heapType) {
		BDBG_MSG(("found mem %d (%p): type 0x%x, heapType %0x, size 0x%x\n", i,
                      clientConfig.heap[i], s.memoryType, s.heapType, s.size));
		*heap = clientConfig.heap[i];
		break;
	}
    }
    if (NEXUS_MAX_HEAPS == i) {
	retValue = B_ERROR_UNKNOWN;
    }
	return retValue;
}


static B_Error B_Secbuf_P_Alloc(size_t size, B_Secbuf_Type type, void * token, void ** buffer)
{
	unsigned int retValue = B_ERROR_SUCCESS;
    B_Secbuf_Desc_T desc = NULL;
    void * ptr = NULL;
    void * ptr_generic = NULL;
    NEXUS_MemoryAllocationSettings memSettings;
    B_Secbuf_Desc_Priv_T priv = NULL;
    NEXUS_MemoryBlockHandle block = NULL;
    BERR_Code rc;

    BDBG_MSG(("Entering %s %u byptes", __FUNCTION__, size));

    if (NULL == g_secbuf) {
        retValue = B_Secbuf_Open();
    }
    if (retValue != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s(%d): B_Secbuf_Open failed", __FUNCTION__, __LINE__));
        retValue = B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    if (!size) {
        BDBG_ERR(("%s(%d): zero size", __FUNCTION__, __LINE__));
        retValue = B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    if (size > g_max_pkt) {
        BDBG_ERR(("%s(%d): large packet %u > max pkt %u, abort", __FUNCTION__, __LINE__, size, g_max_pkt));
        retValue = B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    desc = BKNI_Malloc(sizeof(struct B_Secbuf_Desc));
    if (!desc) {
        BDBG_ERR(("%s(%d): mem alloc error, type %u", __FUNCTION__, __LINE__));
        retValue = B_ERROR_OUT_OF_MEMORY;
        goto out;
    }
    BKNI_Memset(desc, 0, sizeof(struct B_Secbuf_Desc));

    priv = BKNI_Malloc(sizeof(struct B_Secbuf_Desc_Priv));
    if (!priv) {
        BDBG_ERR(("%s(%d): mem alloc error, type %u", __FUNCTION__, __LINE__));
        retValue = B_ERROR_OUT_OF_MEMORY;
        goto out;
    }
    BKNI_Memset(priv, 0, sizeof(struct B_Secbuf_Desc_Priv));

    if (g_secbuf->dma_buf_len < size) {
        /* re-allocate dma_buf */
        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
        memSettings.heap = g_secbuf->settings.generic_heap;
        if (g_secbuf->dma_buf) {
            NEXUS_Memory_Free(g_secbuf->dma_buf);
            g_secbuf->bytes_alloc -= g_secbuf->dma_buf_len;
        }
        NEXUS_Memory_Allocate(size, &memSettings, (void *)&g_secbuf->dma_buf);
        if (!g_secbuf->dma_buf) {
            BDBG_ERR(("%s(%d): mem alloc error, type %u, size %u", __FUNCTION__, __LINE__, size));
            retValue = B_ERROR_OUT_OF_MEMORY;
            goto out;
        }
        g_secbuf->dma_buf_len = size;
        g_secbuf->bytes_alloc += g_secbuf->dma_buf_len;
    }
    if (type == B_Secbuf_Type_eGeneric) {
        if (token) {
            /* use existing buffer */
            block = NEXUS_MemoryBlock_Clone (token);
            if (!block) {
                BDBG_ERR(("%s(%d): token alloc error, type %u", __FUNCTION__, __LINE__, type));
                retValue = B_ERROR_OUT_OF_MEMORY;
                goto out;
            }
            rc = NEXUS_MemoryBlock_Lock(block, &ptr_generic);
            if(rc!=NEXUS_SUCCESS) {
                BDBG_ERR(("%s(%d): NEXUS_MemoruBlock_Lock error", __FUNCTION__, __LINE__));
                retValue = B_ERROR_OUT_OF_MEMORY;
                goto out;
            }

            desc->token = NULL; /* token used */
            g_secbuf->bytes_shared += size;
        } else {
            ptr_generic = B_Secbuf_P_Memory_Allocate(desc, size, B_Secbuf_Type_eGeneric);
            if (!ptr_generic) {
                BDBG_ERR(("%s(%d): mem alloc error, type %u", __FUNCTION__, __LINE__, type));
                retValue = B_ERROR_OUT_OF_MEMORY;
                goto out;
            }
            g_secbuf->bytes_alloc += size;
        }
        priv->addr_cache = ptr_generic;
        desc->addr = ptr_generic;
        BDBG_MSG(("priv %p, addr_cache %p", priv, priv->addr_cache));
    } else if (type == B_Secbuf_Type_eSecure) {
        int i = 0;

        ptr_generic = BKNI_Malloc(size);
        if (!ptr_generic) {
            BDBG_ERR(("%s(%d): mem alloc error, type %u", __FUNCTION__, __LINE__, type));
            retValue = B_ERROR_OUT_OF_MEMORY;
            goto out;
        }
        priv->addr_cache = ptr_generic;
        BDBG_MSG(("priv %p, addr_cache %p", priv, priv->addr_cache));

        if (token) {
            BDBG_MSG(("use existing buffer from token %p", token));
            block = NEXUS_MemoryBlock_Clone (token);
            if (!block) {
                BDBG_ERR(("%s(%d): token alloc error, type %u", __FUNCTION__, __LINE__, type));
                retValue = B_ERROR_OUT_OF_MEMORY;
                goto out;
            }
            rc = NEXUS_MemoryBlock_Lock(block, &ptr);
            if (!ptr || rc!=NEXUS_SUCCESS) {
                BDBG_ERR(("%s(%d): token alloc error, type %u", __FUNCTION__, __LINE__, type));
                retValue = B_ERROR_OUT_OF_MEMORY;
                goto out;
            }
            desc->token = NULL; /* token used */
            desc->addr = ptr;
            g_secbuf->bytes_shared += size;

        } else {
            if (use_secure_mem) {
                ptr = B_Secbuf_P_Memory_Allocate(desc, size, B_Secbuf_Type_eSecure);
            } else {
                ptr = B_Secbuf_P_Memory_Allocate(desc, size, B_Secbuf_Type_eGeneric);
            }
            if (!ptr) {
                BDBG_ERR(("%s(%d): mem alloc error, type %u, size %u", __FUNCTION__, __LINE__, type, size));
                retValue = B_ERROR_OUT_OF_MEMORY;
                goto out;
            }
            BDBG_MSG(("%s(%d): desc->addr mem %p, size %u", __FUNCTION__, __LINE__, ptr, size));
            desc->addr = ptr;
            g_secbuf->bytes_alloc += size;
        }
        priv->dma_blocks = BKNI_Malloc(sizeof(NEXUS_DmaJobBlockSettings) * DEFAULT_NUM_DMA_BLOCKS);
        if (!priv->dma_blocks) {
            BDBG_ERR(("%s(%d): mem alloc error", __FUNCTION__, __LINE__));
            retValue = B_ERROR_OUT_OF_MEMORY;
            goto out;
        }
        BKNI_Memset(priv->dma_blocks, 0, sizeof(NEXUS_DmaJobBlockSettings) * DEFAULT_NUM_DMA_BLOCKS);
        priv->num_dma_blocks = DEFAULT_NUM_DMA_BLOCKS;
        for ( i = 0; i < priv->num_dma_blocks; i++) {
            NEXUS_DmaJob_GetDefaultBlockSettings(&priv->dma_blocks[i]);
        }
    } else {
        BDBG_ERR(("%s(%d): unknown type %u", __FUNCTION__, __LINE__, type));
        retValue == B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    desc->len = size;
    desc->priv = priv;
    desc->type = type;
    desc->magic = B_SECBUF_MAGIC;
    desc->free_callback = B_Secbuf_Free_Callback;
    BKNI_AcquireMutex(g_secbuf->desc_list_mutex);
    if (!find_desc(desc->addr)) {
        BLST_AA_TREE_INSERT(_B_Secbuf_List_Head, &g_secbuf->desc_list, desc->addr, desc);
    } else {
        BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
        BDBG_ERR(("%s(%d): addr %p already in list", __FUNCTION__, __LINE__, desc->addr));
        retValue = B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
    BDBG_MSG(("%s(%d): insert %p/%p to list", __FUNCTION__, __LINE__, desc, desc->addr));
    *buffer = desc->addr;
    if (size > g_secbuf->bytes_packet_max) {
        g_secbuf->bytes_packet_max = size;
    }
 out:
    if (retValue) {
        if (ptr_generic) {
            if (type == B_Secbuf_Type_eGeneric) {
                NEXUS_Memory_Free(ptr_generic);
            } else if (type == B_Secbuf_Type_eSecure) {
                BKNI_Free(ptr_generic);
            }
        }
        if (ptr) NEXUS_Memory_Free(ptr);
        if (priv) BKNI_Free(priv);
        if (desc->token) NEXUS_MemoryBlock_DestroyToken(desc->token);
        if (desc) BKNI_Free(desc);
    }
    {
        static int count = 0;
        g_secbuf->bytes_alloc_max =  (g_secbuf->bytes_alloc_max < g_secbuf->bytes_alloc) ? g_secbuf->bytes_alloc : g_secbuf->bytes_alloc_max;
        g_secbuf->bytes_shared_max =  (g_secbuf->bytes_shared_max < g_secbuf->bytes_shared) ? g_secbuf->bytes_shared : g_secbuf->bytes_shared_max;
        if ((count % 5000) == 1) {
            BDBG_LOG(("secbuf allocated (cur/max) %u/%u, shared(cur/max) %u/%u , max packet %u", g_secbuf->bytes_alloc, g_secbuf->bytes_alloc_max,
                      g_secbuf->bytes_shared, g_secbuf->bytes_shared_max, g_secbuf->bytes_packet_max));
            if (count > 100000) count = 0;
        }
        count++;
    }
    BDBG_MSG(("Leaving %s %p", __FUNCTION__, desc));
    return retValue;
}

B_Error B_Secbuf_Alloc(size_t size, B_Secbuf_Type type, void ** buffer)
{
	unsigned int retValue = B_ERROR_SUCCESS;

    *buffer = NULL;
    if (size > g_max_pkt) {
        BDBG_ERR(("%s(%d): large packet %u > max pkt %u, abort", __FUNCTION__, __LINE__, size, g_max_pkt));
        retValue = B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    retValue = B_Secbuf_P_Alloc(size, type, NULL, buffer);
 out:
    return retValue;
}

B_Error B_Secbuf_AllocWithToken(size_t size, B_Secbuf_Type type, B_SecbufToken token, void ** buffer)
{
	unsigned int retValue = B_ERROR_SUCCESS;
    BDBG_MSG(("Entering %s %u byptes token %p", __FUNCTION__, size, token));
    if (! token) {
        BDBG_ERR(("%s(%d) null token", __FUNCTION__, __LINE__));
        goto out;
    }
    if (size > g_max_pkt) {
        BDBG_ERR(("%s(%d): large packet %u > max pkt %u, abort", __FUNCTION__, __LINE__, size, g_max_pkt));
        retValue = B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    retValue = B_Secbuf_P_Alloc(size, type, token, buffer);
    BDBG_MSG(("Leaving %s %p", __FUNCTION__, *buffer));
 out:
    return retValue;
}

void B_Secbuf_Free_Callback( void * arg)
{
    B_Secbuf_Free(arg);
}
B_Error B_Secbuf_Free(void * buffer)
{
	unsigned int retValue = B_ERROR_SUCCESS;
    B_Secbuf_Desc_T desc = NULL;
    B_Secbuf_Desc_Priv_T priv = NULL;

    if (! buffer) goto out;
    BDBG_MSG(("Entering %s %p", __FUNCTION__, buffer));
    BKNI_AcquireMutex(g_secbuf->desc_list_mutex);
    desc = find_desc(buffer);
    if (!desc) {
        BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
        BDBG_ERR(("Cannot find %p in list", buffer));
        goto out;
    }
    BLST_AA_TREE_REMOVE(_B_Secbuf_List_Head, &g_secbuf->desc_list, desc);
    BDBG_MSG(("%s(%d): remove %p/%p from list", __FUNCTION__, __LINE__, desc, desc->addr));

    BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);

    priv = (B_Secbuf_Desc_Priv_T) desc->priv;
    if (priv->addr_cache) {
        if (desc->type == B_Secbuf_Type_eGeneric) {
            NEXUS_Memory_Free(priv->addr_cache);
            g_secbuf->bytes_alloc -= desc->len;
        } else if (desc->type == B_Secbuf_Type_eSecure) {
            BKNI_Free(priv->addr_cache);
        }
    }
    if (desc->type == B_Secbuf_Type_eSecure) {
        NEXUS_Memory_Free(desc->addr);
    }
    if (desc->token) {
        NEXUS_MemoryBlock_DestroyToken(desc->token);
        if (desc->type == B_Secbuf_Type_eSecure)
            g_secbuf->bytes_alloc -= desc->len;
    } else {
        g_secbuf->bytes_shared -= desc->len;
    }
    if (priv) {
        if (priv->dma_blocks) {
            BKNI_Free(priv->dma_blocks);
            priv->dma_blocks = NULL;
        }
        BKNI_Free(priv);
    }
    BKNI_Free(desc);
 out:
    BDBG_MSG(("Leaving %s", __FUNCTION__));
    return retValue;
}



B_Error B_Secbuf_ImportData(void * buffer, uint32_t offset, unsigned char * pDataIn, size_t len, bool last)
{
	unsigned int retValue = B_ERROR_SUCCESS;
    B_Secbuf_Desc_Priv_T priv = NULL;
    B_Secbuf_Desc_T desc;
    if (!len && !last) {
        BDBG_MSG(("%s: len = %u, ignored", __FUNCTION__, len));
        goto out;
    }
    BKNI_AcquireMutex(g_secbuf->desc_list_mutex);
    desc = find_desc(buffer);
    BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
    if (!desc) {
        BDBG_ERR(("Cannot find %p in list", buffer));
        goto out;
    }
    BDBG_MSG(("Entering %s %p offset %u len %u priv %p", __FUNCTION__, buffer, offset, len , desc->priv));

    priv = desc->priv;
    if (desc->type == B_Secbuf_Type_eGeneric) {
        BKNI_Memcpy(desc->addr + offset, pDataIn, len);
        NEXUS_FlushCache(desc->addr + offset, len);
    } else  if (desc->type == B_Secbuf_Type_eSecure) {
        unsigned char * dma_buf = NULL;
        NEXUS_DmaJobBlockSettings *pBlockSettings;
        BKNI_AcquireMutex(g_secbuf->dma_mutex);
        if (len) {
            dma_buf = priv->addr_cache + offset;
            /* use interm buffer to copy */
            BKNI_Memcpy(dma_buf, pDataIn, len);
            if (priv->cur_dma_blocks < priv->num_dma_blocks) {
                pBlockSettings = &priv->dma_blocks[priv->cur_dma_blocks++];
            } else {
                BDBG_ERR(("Out of dma blocks, max (%u)", priv->num_dma_blocks));
                retValue == B_ERROR_INVALID_PARAMETER;
                BKNI_ReleaseMutex(g_secbuf->dma_mutex);
                goto out;
            }
            pBlockSettings->pSrcAddr = dma_buf;
            pBlockSettings->pDestAddr = desc->addr + offset;
            pBlockSettings->blockSize = len;
            pBlockSettings->cached = false;
        }
        if (last && priv->cur_dma_blocks) {
            retValue = B_Secbuf_P_DoDma(desc);
        }
        BKNI_ReleaseMutex(g_secbuf->dma_mutex);
    } else  {
        BDBG_ERR(("Unknown type %d", desc->type));
        retValue == B_ERROR_INVALID_PARAMETER;
        goto out;
    }
 out:
    BDBG_MSG(("Leaving %s", __FUNCTION__));
    return retValue;
}

static B_Error B_Secbuf_P_DoDma(B_Secbuf_Desc_T desc )
{
	unsigned int retValue = B_ERROR_UNKNOWN;
    NEXUS_Error rc;

    NEXUS_DmaJobBlockSettings *pBlockSettings = NULL;
    BDBG_MSG(("Entering %s", __FUNCTION__));
    B_Secbuf_Desc_Priv_T priv = desc->priv;
    int i = 0;

    pBlockSettings = priv->dma_blocks;

    for ( i = 0; i < priv->cur_dma_blocks; i++) {
        if (desc->type == B_Secbuf_Type_eSecure) {
            uint8_t *dma_buf, *src_addr;
            /* replace src buf with dma_buf */
            src_addr = (uint8_t *) pBlockSettings[i].pSrcAddr;
            dma_buf = g_secbuf->dma_buf + (src_addr - priv->addr_cache) ;
            BKNI_Memcpy(dma_buf, pBlockSettings[i].pSrcAddr, pBlockSettings[i].blockSize);
            pBlockSettings[i].pSrcAddr = dma_buf;
        }
        NEXUS_FlushCache(pBlockSettings[i].pSrcAddr, pBlockSettings[i].blockSize);
    }
    if (desc->type == B_Secbuf_Type_eGeneric) {
        retValue = B_ERROR_SUCCESS;
        goto out;
    }

    rc = NEXUS_DmaJob_ProcessBlocks(g_secbuf->dmaJob, pBlockSettings, priv->cur_dma_blocks);
    if (rc == NEXUS_DMA_QUEUED) {
        for (;;) {
            NEXUS_DmaJobStatus status;
            rc = NEXUS_DmaJob_GetStatus(g_secbuf->dmaJob, &status);
            if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("DmaJob_GetStatus err=%d\n", __FUNCTION__,__LINE__, rc));
                retValue = B_ERROR_OS_ERROR;
                goto out;
            }
            if (status.currentState == NEXUS_DmaJobState_eComplete ) {
                break;
            }
            BKNI_Delay(1);
        }
    }
    else {
        if (rc)
	    BDBG_ERR(("%s(%d):error in dma transfer, err:%d\n", __FUNCTION__, __LINE__, rc));
        retValue = rc;
        goto out;
    }
	retValue = B_ERROR_SUCCESS;
 out:
    for ( i = 0; i < priv->num_dma_blocks; i++) {
        NEXUS_DmaJob_GetDefaultBlockSettings(&priv->dma_blocks[i]);
    }
    priv->cur_dma_blocks = 0;
    BDBG_MSG(("Leaving %s", __FUNCTION__));
    return retValue;
}
static B_Error B_Secbuf_P_CopyData(B_Secbuf_Desc_T desc, unsigned char * pDataOut, unsigned char * pDataIn, size_t len, int flush)
{
	unsigned int retValue = B_ERROR_UNKNOWN;
    NEXUS_Error rc;

    NEXUS_DmaJobBlockSettings blockSettings;
    BDBG_MSG(("Entering %s in %p out %p len %u", __FUNCTION__, pDataIn, pDataOut, len));
    if (len > desc->len ) {
        BDBG_ERR(("DMA size %u too large, max %u", len, desc->len));
        BDBG_ERR(("in/out/flush %p/%p/%d", pDataIn, pDataOut, flush));
        retValue == B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);

    blockSettings.pSrcAddr = pDataIn;
    blockSettings.pDestAddr = pDataOut;
    blockSettings.blockSize = len;

    blockSettings.cached = false;

    if (!len) {
        BDBG_ERR(("%s: len = %u, ignored", __FUNCTION__, len));
        retValue = B_ERROR_SUCCESS;
        goto out;
    }

    /* BDBG_ERR(("%s pDataOut %p pDataIn %p, len %u, flush %u", __FUNCTION__, pDataOut, pDataIn, len, flush ));   */

    if (flush) {
        NEXUS_FlushCache(blockSettings.pSrcAddr, blockSettings.blockSize);
    }

    rc = NEXUS_DmaJob_ProcessBlocks(g_secbuf->dmaJob, &blockSettings, 1);
    if (rc == NEXUS_DMA_QUEUED) {
        for (;;) {
            NEXUS_DmaJobStatus status;
            rc = NEXUS_DmaJob_GetStatus(g_secbuf->dmaJob, &status);
            if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("DmaJob_GetStatus err=%d\n", __FUNCTION__,__LINE__, rc));
                retValue = B_ERROR_OS_ERROR;
                goto out;
            }
            if (status.currentState == NEXUS_DmaJobState_eComplete ) {
                break;
            }
            BKNI_Delay(1);
        }
    }
    else {
        if (rc) {
            BDBG_ERR(("%s(%d):error in dma transfer, rc:%d\n", __FUNCTION__, __LINE__, rc));
            goto out;
        }
    }

    if (flush) {
        NEXUS_FlushCache(blockSettings.pSrcAddr, blockSettings.blockSize);
    }
    retValue = B_ERROR_SUCCESS;
 out:
    BDBG_MSG(("Leaving %s", __FUNCTION__));
    return retValue;
}

B_Error B_Secbuf_ExportData(void * buffer, unsigned int offset, unsigned char * pDataOut, size_t len, bool last)
{
    B_Secbuf_Desc_T desc = NULL;
    unsigned int retValue = B_ERROR_SUCCESS;
    if (!len && !last) {
        BDBG_MSG(("%s: len = %u, ignored", __FUNCTION__, len));
        goto out;
    }
    BKNI_AcquireMutex(g_secbuf->desc_list_mutex);
    desc = find_desc(buffer);
    BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
    if (!desc) {
        BDBG_ERR(("Cannot find %p in list", buffer));
        goto out;
    }
    BDBG_MSG(("%s desc->addr %p, pDataOut %p", __FUNCTION__, desc->addr + offset, pDataOut ));
    BKNI_AcquireMutex(g_secbuf->dma_mutex);
    /* for now just do one dma, should really use dma blocks */
    if (last) {
        retValue = B_Secbuf_P_CopyData(desc, pDataOut, desc->addr + offset, len, (desc->type == B_Secbuf_Type_eSecure) ? 0 : 1); //no flush for eSecure buffer
    }
    if (retValue != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s(%d): desc/offset/len-%p/x%x/%u", __FUNCTION__, __LINE__, desc, offset, len));
    }
    BKNI_ReleaseMutex(g_secbuf->dma_mutex);
 out:
    return retValue;
}

B_Error B_Secbuf_ImportDataChunk(void * buffer, unsigned char * pDataIn, void * chunk_info, unsigned int chunk_count)
{
	unsigned int retValue = B_ERROR_SUCCESS;
    unsigned int i = 0;
    unsigned int o_offset = 0;
    unsigned int i_offset = 0;
    B_Secbuf_Desc_T desc = NULL;
    struct _chunk_info * ci = (struct _chunk_info *) chunk_info;
    BDBG_MSG(("Entering %s %p count %u", __FUNCTION__, buffer, chunk_count));
    if (!chunk_count) {
        BDBG_ERR(("%s: chunk_count = %u, ignored", __FUNCTION__, chunk_count));
        goto out;
    }
    if (!pDataIn) {
        BDBG_ERR(("%s: NULL pointer pDataIn %p", __FUNCTION__, pDataIn));
        retValue = B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    BKNI_AcquireMutex(g_secbuf->desc_list_mutex);
    desc = find_desc(buffer);
    BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
    if (!desc) {
        BDBG_ERR(("Cannot find %p in list", buffer));
        goto out;
    }
    if (desc->type == B_Secbuf_Type_eSecure || desc->type == B_Secbuf_Type_eGeneric) {
        unsigned char * dma_buf = NULL;

        BKNI_AcquireMutex(g_secbuf->dma_mutex);
        for(i = 0; i < chunk_count; i++) {
            o_offset += ci[i].clear_size;
            if ( ci[i].encrypted_size) {
                retValue = B_Secbuf_P_CopyData(desc, desc->addr + o_offset, pDataIn + i_offset, ci[i].encrypted_size, 0);
                if (retValue != B_ERROR_SUCCESS) {
                    BDBG_ERR(("%s(%d): desc/dest/src/len-%p/x%x/x%x/%u", __FUNCTION__, __LINE__,
                              desc, desc->addr + o_offset, pDataIn + i_offset, ci[i].encrypted_size));
                }
            }
            o_offset += ci[i].encrypted_size;
            i_offset += ci[i].encrypted_size;
        }
        BDBG_MSG(("%s(%d): dest 0x%x, size %u", __FUNCTION__, __LINE__, desc->addr, o_offset));
        BKNI_ReleaseMutex(g_secbuf->dma_mutex);

    } else  {
        BDBG_ERR(("Unknown type %d", desc->type));
        retValue == B_ERROR_INVALID_PARAMETER;
        goto out;
    }
    retValue = B_ERROR_SUCCESS;
 out:
    BDBG_MSG(("Leaving %s", __FUNCTION__));
    return retValue;
}

B_Error B_Secbuf_GetBufferInfo(void * buffer, B_Secbuf_Info * info)
{
    B_Secbuf_Desc_T desc = NULL;
	unsigned int retValue = B_ERROR_INVALID_PARAMETER;
    BKNI_AcquireMutex(g_secbuf->desc_list_mutex);
    desc = find_desc(buffer);
    BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
    if (!desc) {
        BDBG_ERR(("Cannot find %p in list", buffer));
        goto out;
    }
    if (desc) {
        info->type = desc->type;
        info->size = desc->len;
        info->token = desc->token;
        retValue = B_ERROR_SUCCESS;

    }
    retValue = B_ERROR_SUCCESS;
 out:
    return retValue;
}
/* This API frees the internal secbuf data structure associated to buffer.
 * but the token is not freed, which can be passed to another process.
 * Token can be re-used by  B_Secbuf_AllocWithToken().
 *
*/

B_Error B_Secbuf_FreeDesc(void * buffer)
{
	unsigned int retValue = B_ERROR_INVALID_PARAMETER;
    B_Secbuf_Desc_Priv_T priv = NULL;
    B_Secbuf_Desc_T desc = NULL;

    if (! buffer) goto out;
    BDBG_MSG(("Entering %s %p", __FUNCTION__, buffer));
    BKNI_AcquireMutex(g_secbuf->desc_list_mutex);
    desc = find_desc(buffer);
    if (!desc) {
        BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
        BDBG_ERR(("Cannot find %p in list", buffer));
        goto out;
    }
    BLST_AA_TREE_REMOVE(_B_Secbuf_List_Head, &g_secbuf->desc_list, desc);
    BKNI_ReleaseMutex(g_secbuf->desc_list_mutex);
    BDBG_MSG(("%s(%d): remove %p/%p from list", __FUNCTION__, __LINE__, desc, desc->addr));

    priv = (B_Secbuf_Desc_Priv_T) desc->priv;
    if (desc->type == B_Secbuf_Type_eSecure) {
        if (priv) {
            if (priv->addr_cache) {
                BKNI_Free(priv->addr_cache);
				priv->addr_cache = NULL;
            }
            if (priv->dma_blocks) {
                BKNI_Free(priv->dma_blocks);
                priv->dma_blocks = NULL;
            }
            /* free desc->addr but do not free token */
            if (desc->addr) {
                NEXUS_Memory_Free(desc->addr);
                g_secbuf->bytes_alloc -= desc->len;
            }
            BKNI_Free(priv);
        }
    } else if (desc->type == B_Secbuf_Type_eGeneric) {
        if (priv) {
            BKNI_Free(priv);
        }
    }
    BKNI_Free(desc);
    retValue = B_ERROR_SUCCESS;
 out:
    BDBG_MSG(("Leaving %s", __FUNCTION__));
    return retValue;
}

static uint8_t * B_Secbuf_P_Memory_Allocate(B_Secbuf_Desc_T desc, uint32_t size, B_Secbuf_Type memoryType)
{
    uint8_t * pMemory = NULL;
    NEXUS_MemoryBlockHandle block;
    NEXUS_MemoryBlockTokenHandle token;
    NEXUS_Error rc;

    block = NEXUS_MemoryBlock_Allocate((memoryType == B_Secbuf_Type_eSecure) ? g_secbuf->settings.secure_heap : g_secbuf->settings.generic_heap,
                                       size, 0, NULL);
    rc = NEXUS_MemoryBlock_Lock(block, &pMemory);
    if (rc) {
        pMemory = NULL;
        BERR_TRACE(rc);
    }
    /* clone the block */
    token = NEXUS_MemoryBlock_CreateToken(block);
    if (!token) {
        BDBG_ERR(("clone failed"));
        NEXUS_Memory_Free(pMemory);
        pMemory = NULL;
        goto out;
    }
    desc->token = (void *) token;
    BDBG_MSG(("token %p created", desc->token));

 out:
    return pMemory;
}
