/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include <stdio.h>
#include "bsu_stdlib.h"
#include "bsu_memory.h"
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
