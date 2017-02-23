/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
***************************************************************************/
#include "nexus_base.h"
#include "nexus_platform_priv.h"
#include "../common/ipc/nexus_ipc_server_api.h"
#include "nexus_client_resources.h"
BDBG_MODULE(nexus_platform_ipc);

NEXUS_Error NEXUS_P_ServerCall_InVarArg_AddrField(unsigned in_data_size, void *in_data, unsigned struct_size, unsigned field_offset, unsigned count, int varArg, int varArgField)
{
    if(varArg!=-1) {
        void *varArgData = (uint8_t *)in_data  + varArg;
        const NEXUS_Addr *dataAddr = (void *)((uint8_t *)in_data  + varArgField);
        unsigned i;
        if(varArg + count * struct_size > in_data_size) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if(varArgField + count * sizeof(NEXUS_Addr) > in_data_size) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        for(i=0;i<count;i++) {
            BDBG_MSG(("NEXUS_P_ServerCall_InVarArg_AddrField:%u %d(%d) %p[%u]=" BDBG_UINT64_FMT "", field_offset, varArg, varArgField, (void *)dataAddr, i, BDBG_UINT64_ARG(dataAddr[i])));
            *(void **)((uint8_t *)varArgData + i*struct_size + field_offset) = NEXUS_P_ServerCall_OffsetToAddr(dataAddr[i]);
        }
    }
    return NEXUS_SUCCESS;
}

NEXUS_Addr NEXUS_P_ServerCall_AddrToOffset(const void *ptr)
{
    if(ptr!=NULL) {
        NEXUS_Addr addr = NEXUS_AddrToOffset(ptr);
        if(addr==0) {
            (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        return addr;
    }
    return 0;
}

void *NEXUS_P_ServerCall_OffsetToAddr(NEXUS_Addr addr)
{
    if(addr!=0) {
        return NEXUS_OffsetToCachedAddr(addr);
    }
    return NULL;
}

void NEXUS_P_ServerCall_OutVarArg_Init(NEXUS_P_ServerOutVarArg_State *state, struct b_objdb_client *client, void *data, unsigned size)
{
    state->data = data;
    state->original_data = data;
    state->client = client;
    state->header = 0;
    state->varargs_begin = 0;
    state->varargs_offset = 0;
    state->size = size;
    return;
}

NEXUS_Error NEXUS_P_ServerCall_OutVarArg_Allocate(NEXUS_P_ServerOutVarArg_State *state, unsigned total_data_size)
{
    unsigned aligned_data_size = B_IPC_DATA_ALIGN(total_data_size);
    unsigned new_size = state->header + state->varargs_begin + state->varargs_offset + aligned_data_size;
    if(new_size > state->size) {
        void *new_out = nexus_client_driver_alloc(state->client, new_size );
        BDBG_MSG(("OutVarArg_Allocate: %u(%p) -> %u(%p)", state->size, state->data, new_size, new_out));
        if(new_out==NULL) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        BKNI_Memcpy(new_out, state->data, state->varargs_begin + state->varargs_offset + state->header);
        if(state->data != state->original_data) {
            BKNI_Free(state->data);
        }
        state->data = new_out;
        state->size = new_size;
    }
    return NEXUS_SUCCESS;
}

void *NEXUS_P_ServerCall_OutVarArg_Place(NEXUS_P_ServerOutVarArg_State *state, unsigned data_size, int *field_offset)
{
    unsigned aligned_data_size = B_IPC_DATA_ALIGN(data_size);
    unsigned new_size = state->header + state->varargs_begin + state->varargs_offset + aligned_data_size;
    void *data;
    *field_offset = -1;
    if(new_size > state->size) {
        (void)BERR_TRACE(NEXUS_UNKNOWN);
        return NULL;
    }
    *field_offset = state->varargs_offset +  state->varargs_begin;
    data = (uint8_t *)state->data + state->header + state->varargs_begin + state->varargs_offset;
    state->varargs_offset += aligned_data_size;
    return data;
}

void NEXUS_P_ServerCall_OutVarArg_Shutdown(NEXUS_P_ServerOutVarArg_State *state)
{
    if(state->data != state->original_data) {
        BKNI_Free(state->data);
    }
    return;
}

NEXUS_Error NEXUS_Platform_GetHeapStatus_driver(NEXUS_HeapHandle heap, NEXUS_MemoryStatus *pStatus)
{
    return NEXUS_Heap_GetStatus_driver_priv(heap, pStatus);
}
