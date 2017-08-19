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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_transport_module.h"
#include "bioatom.h" /* needed for batom_vec */
#include <stdio.h>

BDBG_MODULE(nexus_playpump_external_crypto);

#define BDBG_MSG_FLOW(X)

void *NEXUS_PlaypumpExternalCrypto_Start( void *securityContextData )
{
    BSTD_UNUSED(securityContextData);
    /* TODO: open custom crypto interface here */
    return (void *)1; /* must be non-zero, even if unused */
}

static NEXUS_Error decrypt( void *handle, void *addr, unsigned len )
{
    BSTD_UNUSED(handle);
    /* if NEXUS_PlaypumpExternalCrypto_Decrypt is right, then this data is always whole packets */
    BDBG_ASSERT(len % 188 == 0);

    BDBG_MSG(("%s(): (addr=0x%x(%d), len=%d)", BSTD_FUNCTION, (unsigned)addr, (unsigned)addr, len));
    if (len == 0)
        return 0;

    if (((char*)addr)[0] != 0x47) {
        printf("\n not aligned to TS packet !! \n\n");
    }

    /* TODO: add custom crypto call here. must modify in-place. */

    return 0;
}


NEXUS_Error NEXUS_PlaypumpExternalCrypto_Decrypt( void *handle, const batom_vec *vec_array, unsigned vec_array_entries )
{
/*
    vec_array[0].base[0] is the start of a packet
    vec_array[vec_array_entries-1].base[vec_array[vec_array_entries-1].len-1] is the end of a packet
    the following code is an example of how to convert this into 188-byte packet calls using a temp packet
*/
    unsigned i;
    unsigned char temp[188];
    unsigned temp_size = 0;
    NEXUS_Error rc = 0;

    BDBG_MSG(("decrypt %p %d", vec_array, vec_array_entries));

    for (i=0;i<vec_array_entries;i++) {
        uint8_t *thisaddr;
        unsigned thissize;
        unsigned remainder;
        uint8_t *copybackaddr;
        unsigned copybacksize;

        thisaddr = vec_array[i].base;
        thissize = vec_array[i].len;

        BDBG_MSG_FLOW(("[%d] base=0x%x(%d), len = %d", i, (unsigned)thisaddr, (unsigned)thisaddr, thissize));

        NEXUS_FlushCache(thisaddr, thissize);

        /* if we have data in the temp buffer from previous vec_array, try to complete it now */
        if (temp_size) {
            if (temp_size + vec_array[i].len >= 188) {
                unsigned n = 188 - temp_size;
                /* we can complete it */
                BKNI_Memcpy(&temp[temp_size], vec_array[i].base, n);
                rc = decrypt(handle, temp, 188);
                if (rc) return BERR_TRACE(rc);

                /* copy back decrypted data to vec_arrary */
                if (i>0) {
                    copybackaddr = (uint8_t*)(vec_array[i-1].base) + vec_array[i-1].len - temp_size;
                    copybacksize = temp_size;
                    BDBG_MSG_FLOW(("[%d] copy back addr=0x%x(%d), len = %d", i, (unsigned)copybackaddr, (unsigned)copybackaddr, copybacksize));
                    BKNI_Memcpy(copybackaddr, temp, copybacksize);

                    copybackaddr = (uint8_t*)(vec_array[i].base);
                    copybacksize = n;
                    BDBG_MSG_FLOW(("[%d] copy back addr=0x%x(%d), len = %d", i, (unsigned)copybackaddr, (unsigned)copybackaddr, copybacksize));
                    BKNI_Memcpy(copybackaddr, &temp[temp_size], copybacksize);
                }

                temp_size = 0;
                thisaddr += n;
                thissize -= n;
            }
            else {
                /* can't complete the temp packet yet */
                BKNI_Memcpy(&temp[temp_size], vec_array[i].base, vec_array[i].len);
                temp_size += vec_array[i].len;
                continue;
            }
        }

        /* decrypt all data up to the last packet */
        remainder = thissize % 188;
        rc = decrypt(handle, thisaddr, thissize - remainder);
        if (rc) return BERR_TRACE(rc);
        BDBG_MSG_FLOW(("[%d] decrypt addr=0x%x(%d), len = %d", i, (unsigned)thisaddr, (unsigned)thisaddr, thissize - remainder));

        if (remainder) {
            BKNI_Memcpy(temp, thisaddr + thissize - remainder, remainder);
            temp_size = remainder;
        }
    }

    if (temp_size) {
        BDBG_ERR(("playpump state machine has failed. external crypto was sent partial packet. crypto failure has occurred."));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    return 0;
}

void NEXUS_PlaypumpExternalCrypto_Stop( void *handle )
{
    BSTD_UNUSED(handle);
    /* TODO: close custom crypto interface here */
}
