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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/***********************************************************************/
/* Header files                                                        */
/***********************************************************************/
#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "bsu_prompt.h"
#include "bsu_nvram.h"
#include "bsu-api.h"
#include "bsu-api2.h"

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/

extern uint64_t xtoq(const char *dest);

#ifdef NO_OS_DIAGS
    extern void bkgdFunc(void *p_generic);
#endif

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

void bcmSataTest (void);

#if __cplusplus
}
#endif

/***********************************************************************
 *                      Global variables
 ***********************************************************************/

 /***********************************************************************
 *
 *  bsu_nvram_test()
 * 
 *  Test BSU nvram functions.
 *
 ***********************************************************************/
void bsu_nvram_test (void)
{
    static char command_id;
    int res;
    static int read_offset=0;
    static int write_offset=0;
    char str[20];
    char *eptr;
    char partition_name[20];
    uint8_t buffer[512];

    strcpy(partition_name, "flash0.nvram");
#ifdef MIPS_SDE
    cfe_set_envdevname(partition_name);
#else
    bolt_set_envdevname(partition_name);
#endif
    res = bsu_nvram_open();

    while (1)
    {
        printf("\n\n");
        printf("================\n");
        printf("  FLASH MENU  \n");
        printf("================\n");
        printf("    0) Exit\n");
        printf("    1) Read from flash partition (starting offset=0x%x)\n", read_offset);
        printf("    2) Write to flash partition (starting offset=0x%x)\n", write_offset);
        printf("    3) Change read starting offset\n");
        printf("    4) Change write starting offset\n");
        printf("    5) Change partition (current partition=%s)\n", partition_name);
        printf("    6) Erase flash\n");

        #ifdef NO_OS_DIAGS
            command_id = BkgdPromptChar( bkgdFunc, (void *) NULL );
        #else
            command_id = PromptChar();
        #endif

        switch(command_id)
        {
            case '0':
                bsu_nvram_close();
                return;

            case '1':
                res = bsu_nvram_read(buffer,read_offset,sizeof(buffer));
                if (res != 0) {
                    unsigned int i;
                    if (res != sizeof(buffer))
                        printf("res=%d, sizeof(buffer)=%d\n", res, sizeof(buffer));
                    for (i=0; i<sizeof(buffer); i++) {
                        if (i%16==0) printf("%08x:  ", read_offset+i);
                        printf("%02x ", buffer[i]); 
                        if (i%16==15) printf("\n");
                    }
                    read_offset += res;
                }
                else {
                    printf("error from nvram_read\n");
                }

                break;

            case '2':
                memset(buffer, 0, sizeof(buffer));
                printf("writing %d bytes at offset 0x%x\n", sizeof(buffer), write_offset);
                res = bsu_nvram_write(buffer,write_offset,sizeof(buffer));
                if (res != 0) {
                    if (res != sizeof(buffer))
                        printf("res=%d, sizeof(buffer)=%d\n", res, sizeof(buffer));
                    write_offset += res;
                }
                else
                    printf("error from nvram_read\n");
                break;

            case '3':
                console_readline("enter read starting offset in hex:  ", str, sizeof(str));
                read_offset = strtoul(str, &eptr, 16);
                break;

            case '4':
                console_readline("enter write starting offset in hex:  ", str, sizeof(str));
                write_offset = strtoul(str, &eptr, 16);
                break;

            case '5':
                printf("enter parition name:  ");
                rdstr(partition_name);
                printf("\nnew partition name=%s\n", partition_name);
                bsu_nvram_close();
#ifdef MIPS_SDE
                cfe_set_envdevname(partition_name);
#else
                bolt_set_envdevname(partition_name);
#endif
                bsu_nvram_open();
                break;

            case '6':
                printf("erasing flash...");
                bsu_nvram_erase();
                break;

            default:
                printf("invalid selection\n");
                break;
        }
    }
}

