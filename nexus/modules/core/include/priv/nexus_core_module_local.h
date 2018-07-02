/***************************************************************************
*  Copyright (C) 2004-2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
***************************************************************************/
#ifndef NEXUS_CORE_MODULE_LOCAL_H__
#define NEXUS_CORE_MODULE_LOCAL_H__

struct NEXUS_MemoryMapNode {
    NEXUS_Addr offset;
    void *lockedMem;
    unsigned size;
    BLST_AA_TREE_ENTRY(NEXUS_P_MemoryMapOffsetTree) offsetNode;
};

struct NEXUS_MemoryBlockLocal {
    BDBG_OBJECT(NEXUS_MemoryBlockLocal) /* not NEXUS_OBJECT */
    BLST_D_ENTRY(NEXUS_MemoryBlockLocal) link;
    BLST_AA_TREE_ENTRY(NEXUS_P_MemoryBlockAddressTree) addressNode;
    NEXUS_MemoryBlockHandle memoryBlock;
    unsigned lockCnt;
    NEXUS_MemoryBlockProperties properties;
    struct NEXUS_MemoryMapNode memoryMap;
};

NEXUS_Error NEXUS_CoreModule_LocalInit(void);
void NEXUS_CoreModule_LocalUninit(void);
void NEXUS_MemoryBlock_Release_local(NEXUS_MemoryBlockHandle memoryBlock, const NEXUS_MemoryBlockUserState *state);
void NEXUS_P_MemoryMap_InitNode(struct NEXUS_MemoryMapNode *node);
NEXUS_Error NEXUS_P_MemoryMap_Map(struct NEXUS_MemoryMapNode *node, NEXUS_Addr offset, size_t size);
void NEXUS_P_MemoryMap_Unmap(struct NEXUS_MemoryMapNode *node, size_t size);


#endif /* NEXUS_CORE_MODULE_LOCAL_H__ */
