/***************************************************************************
*     (c)2012-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_base.h"
#include "nexus_base_stackcheck.h"
#include "nexus_base_priv.h"

BDBG_MODULE(nexus_base_stackcheck);

#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)

BSTD_INLINE unsigned b_crc8_update_uint8(unsigned seed, unsigned data)
{
    unsigned temp;

    /* x8 + x5 + x4 + 1 (CRC-8-Dallas/Maxim) */
    temp = data ^ seed;
    temp ^= (temp << 3) ^ (temp << 4) ^ (temp << 6);
    temp ^=  (temp >> 4) ^ (temp >> 5);
    return temp;
}

BSTD_INLINE unsigned b_crc8_update_uint16(unsigned seed, unsigned data)
{
    seed = b_crc8_update_uint8(seed, data>>8);
    seed = b_crc8_update_uint8(seed, data&0xFF);
    return seed;
}

static unsigned
b_crc8_update_uint32(unsigned seed, unsigned data)
{
    seed = b_crc8_update_uint16(seed, data>>16);
    seed = b_crc8_update_uint16(seed, data&0xFFFF);
    return seed;
}


static unsigned NEXUS_P_Base_StackCheck_Atom_Crc(const void *stack, unsigned atomNo)
{
    unsigned i;
    unsigned seed = 0xFF;
    unsigned long temp = ((unsigned long)stack)+(NEXUS_P_BASE_STACKCHECK_ATOM - 1); /* align up stack boundary */
    const uint32_t *stackTop = (const uint32_t *)(temp - (temp % NEXUS_P_BASE_STACKCHECK_ATOM));
    const uint32_t *atom = stackTop - (atomNo+1)*(NEXUS_P_BASE_STACKCHECK_ATOM/sizeof(uint32_t)); /* stack grows downward */
    for(i=0;i<NEXUS_P_BASE_STACKCHECK_ATOM/sizeof(uint32_t);i++) {
        seed = b_crc8_update_uint32(seed, atom[i]);
    }
    BDBG_MSG_TRACE(("%p..%p:%#x",(void *)atom, (void *)(atom+i), seed));
    return seed&0xFF;
}


void NEXUS_P_Base_StackCheck_MakeFrame(NEXUS_P_Base_StackCheck_Frame *frame, const void *stack)
{
    unsigned i;
    for(i=0;i<NEXUS_P_BASE_STACKSIZE_MIN/NEXUS_P_BASE_STACKCHECK_ATOM;i++) {
        frame->crc[i] = NEXUS_P_Base_StackCheck_Atom_Crc(stack, i);
    }
    return;
}

unsigned NEXUS_P_Base_StackCheck_CheckFrame(const NEXUS_P_Base_StackCheck_Frame *frame, const void *stack)
{
    int i;
    unsigned depth = 0;
    for(i=(NEXUS_P_BASE_STACKSIZE_MIN/NEXUS_P_BASE_STACKCHECK_ATOM)-1;i>=0;i--) {
        unsigned new_frame = NEXUS_P_Base_StackCheck_Atom_Crc(stack, i);
        if(new_frame != frame->crc[i]) {
            BDBG_MSG_TRACE(("%u %#x:%#x", i, frame->crc[i], new_frame));
            depth = i;
            break;
        }
    }
    return (depth+1)*NEXUS_P_BASE_STACKCHECK_ATOM;
}


