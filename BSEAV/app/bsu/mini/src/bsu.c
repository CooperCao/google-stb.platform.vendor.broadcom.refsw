/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 ***************************************************************************/

#include <stdio.h>
#include "bsu_stdlib.h"
#include "bsu_prompt.h"
#include "bsu-api.h"
#include "bsu-api2.h"

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

struct bsu_api *xapi;

/***********************************************************************
 *                      External References
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

extern void mini_exit(void);

#if __cplusplus
}
#endif

/************************************************************************
 *
 *  bcm_main()
 *
 *  This is Broadcom's main function.  It performs some software
 *  initialization and calls the root() functions.
 *
 ************************************************************************/
void bcm_main(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
    struct bsu_meminfo *xfd_mem;
    unsigned int i;
    static char command_id;

    xapi = (struct bsu_api *)r3;
    if (!xapi)
        #if 1
            while (1);
        #else
            _exit2();
        #endif

    xfd_mem = xapi->xfd_mem;

    /* --- now we can use the xapi --- */

    printf("_main(r0:0x%08x r1:0x%08x r2:0x%08x r3:0x%08x)\n",
                    r0, r1, r2, r3);

    printf("BSU @ 0x%08x\n", r3);

    printf("Signature is ");
    if (xapi->xfd_signature == BSU_SIGNATURE) {
        printf("ok\n");
    } else {
        printf("BAD!\n");
        #if 1
            while (1);
        #else
            _exit2();
        #endif
    }

    /* all our free memory to use - NOT including us!
    */
    for(i=0; i < xapi->xfd_num_mem; i++) {
        if (xfd_mem[i].top - xfd_mem[i].base) {

            printf("%d \tmemc #%d 0x%08x -> 0x%08x %dMb\n",
                    i, xfd_mem[i].memc, xfd_mem[i].base, xfd_mem[i].top,
                    (xfd_mem[i].top - xfd_mem[i].base) / (1024*1024));
        }
    }

#if MIPS_BSU_HEAP_SIZE > 0
    /* Once memory has been initialized, we can create the BSU memory heap */
    bsu_heap_init((unsigned char *)MIPS_BSU_HEAP_ADDR, MIPS_BSU_HEAP_SIZE);

//#define TEST_SIZE 8*1024*1024

    #ifdef TEST_SIZE
    {
        void *ptr = malloc(TEST_SIZE);
        if (ptr) {
            bsu_mem_test((unsigned int *)ptr, (unsigned int *)(ptr+TEST_SIZE-1));
            free(ptr);
        } else {
            printf("error allocating memory\n");
        }
    }
    #endif
#endif

    while (1)
    {
        printf( "\n\nMini BSU Test Menu\n\n");

        printf("    1) Option 1\n");
        printf("    2) Option 2\n");
        printf("    3) Exit to BOLT\n");

        command_id = PromptChar();

        switch(command_id)
        {
            case '1':
                printf("Option 1\n");
                break;

            case '2':
                printf("Option 2\n");
                break;

            case '3':
                mini_exit();
                break;

            default:
                printf("Unexpected key pressed\n");
                break;
        }
    }
}

void c_init(void)
{
    extern unsigned int _bss_start;
    extern unsigned int _bss_end;

    unsigned int *bss_start = &_bss_start;
    unsigned int *bss_end = &_bss_end;

    while (bss_start != bss_end)
        *bss_start++ = 0;
}

void OS_CPU_ARM_ExceptIrqHndlr(void)
{
    printf("OS_CPU_ARM_ExceptIrqHndlr stubbed\n");
}
