/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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
#include <string.h>
#include "bstd.h"
#include "breg_mem.h"
#include "prompt.h"
#include "dhrystone.h"

/*---------------------------------------------------------------------*/
/*                      External References                            */
/*---------------------------------------------------------------------*/


#if __cplusplus
extern "C" {
#endif

extern BREG_Handle bcmGetFlashRegHandle (void);
extern void FlashTestMenu(void);
extern void NandFlashTestMenu(void);
extern void xmodem_flash_program_download(BREG_Handle hFlash);
extern void xmodem_sdram_program_download(void);
extern void SdramTestMenu(void);
extern void whetstone_test(void);

#if __cplusplus
}
#endif

/*---------------------------------------------------------------------*/
/*                      Local Functions                                */
/*---------------------------------------------------------------------*/
static void ProgramDownloadMenu(void);
static void ReadWriteMenu(void);
#if 0
static void MemoryProgramBoardParamBlock( void );
static void MemoryDisplayBoardParamBlock( void );
#endif


/*---------------------------------------------------------------------*/
/*                      Local Definitions                              */
/*---------------------------------------------------------------------*/
#define BYTE_SWAP16(x)  ((((x) >> 8) & 0xff) | (((x) << 8) & 0xff00))
#ifndef SWAP_END32
#define SWAP_END32(x) ( (((x)&0xff000000)>>24) |(((x)&0x00ff0000)>>8) | (((x)&0x0000ff00)<<8)  |(((x)&0x000000ff)<<24) )               
#endif
#ifndef SWAP_16 
#define SWAP_16(x) ( (((x)&0xffff0000)>>16) | (((x)&0x0000ffff)<<16) )
#endif


#ifdef __cplusplus
extern "C" {
#endif

void MemoryTestMenu(void);
void CsSwapTestMenu(void);

#ifdef __cplusplus
}
#endif

/****************************************************************
* void MemoryTestMenu(
*   void )
*
* INPUTS:   none
* OUTPUTS:  none
* RETURNS:  none
* FUNCTION: This function is responsible for providing the user
*           interface when the user wants to test/program the
*           various memory devices on this reference design.
*           This function support testing of Flash and SDRAM.
*           It also supports programming of Board Parameter Block
*           into Flash and programming of executable into either
*           SDRAM or Flash.
*
****************************************************************/
void MemoryTestMenu(void)
{
    static unsigned int memory_id;
    int quit;
/*  void (*ProgJumpPtr)(void)=NULL; */

    /* Initialize local variables        */
    quit = 0;
    memory_id = 99;

    /*
     * Loop until quit is 1. quit is set to 1 when
     * 0 is entered.
     */
    while (quit == 0)
    {
        printf("            MEMORY MENU \n");
        printf("    0) Return to Main Menu\n");
        printf("    1) NOR Flash Tests\n");
        printf("    2) NAND Flash Tests\n");
        printf("    3) DMA and non-DMA Memory Tests\n");
        printf("    4) Memory Read/Write\n");
        printf("    5) Program Download\n");
        printf("    6) Dhrystone Test\n");
        printf("    7) Whetstone (FPU) Test\n");
#if 0
        printf("    8) Program Board related info and MAC addr into Flash\n");
        printf("    9) Display Board related info and MAC addr stored in Flash\n");
        printf("    a) NOR/NAND CS SWAP Tests\n");
        printf("    b) AMS-POD interface loop back test.\n");
#endif      

        memory_id = Prompt();
        switch(memory_id)
        {
            case  0:
                quit = 1;
                break;
            case  1:
                #ifdef DIAGS_FLASH_TEST
                FlashTestMenu();
                #else
                printf("Flash Test not currently supported\n");
                #endif
                break;
            case 2:
                #ifdef DIAGS_NAND_FLASH_TEST
                NandFlashTestMenu();
                #else
                printf("NAND Flash Test not currently supported\n");
                #endif
                break;
            case  3:
                #ifdef DIAGS_MEM_DMA_TEST
                    SsoTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;
            case  4:
                ReadWriteMenu();
                break;
            case  5:
                ProgramDownloadMenu();
                break;
            case 6:
                Dhrystone();
                break;
            case 7:
                #ifdef DIAGS_WHETSTONE_TEST
                    whetstone_test();
                #else
                    printf("Not enabled\n");
                #endif
                break;

            #if 0
            case  8:
                MemoryProgramBoardParamBlock();
                break;
            case  9:
                MemoryDisplayBoardParamBlock();
                break;
            case 0xa:
                CsSwapTestMenu();
                break;
            case 0x0b:
            {
                volatile unsigned long *txbuf = (volatile unsigned long*)0xa1811120;
                volatile unsigned long *rxbuf = (volatile unsigned long*)0xa1812120;
                unsigned long size = 0x1000;
                unsigned long i, temp;
                volatile unsigned long *mcif_control = (volatile unsigned long*)0xb0400f00;
                volatile unsigned long *mcif_tx_buf_ptr = (volatile unsigned long*)0xb0400f04;
                volatile unsigned long *mcif_rx_buf_ptr = (volatile unsigned long*)0xb0400f08;
                volatile unsigned long *mcif_tx_len = (volatile unsigned long*)0xb0400f0c;
                volatile unsigned long *mcif_tx_ctrl = (volatile unsigned long*)0xb0400f10;
                volatile unsigned long *mcif_rx_status = (volatile unsigned long*)0xb0400f18;
                volatile unsigned long *mcif_rx_data_status = (volatile unsigned long*)0xb0400f1c;

                volatile unsigned long *mcif_intr2_cpu_status = (volatile unsigned long*)0xb0400f80;
                volatile unsigned long *mcif_intr2_cpu_clear = (volatile unsigned long*)0xb0400f88;


                printf("Please make sure SDI and SDO pins are connected.\n");
                /* initialize mem c priority. */
                *((volatile unsigned long *)0xb01061e0) = 0x000f9c3f;//client info 56
                *((volatile unsigned long *)0xb01061e4) = 0x000f9c3f;//client info 57

                /* set up sundry top mux to enable AMS-POD pins. */
                *((volatile unsigned long*)0xb0404090) &= 0xfffff000;
                *((volatile unsigned long*)0xb0404090) |= 0x492;
                printf("Top mux value 0x%08x. \n", *((volatile unsigned long*)0xb0404090));
                /* set up TOP MUX to enable GPIO 58 */
                *((volatile unsigned long*)0xb0404098) &= 0xffffff9f;//bit 5 and 6 0 -> GPIO58.
                *((volatile unsigned long*)0xb0400728) &= 0xfbffffff; //bit 26 GPIO 58 output.
                *((volatile unsigned long*)0xb0400720) &= 0xfbffffff; // non opendrain.
                *((volatile unsigned long*)0xb0400724) &= 0xfbffffff; // GPIO 58 0.

                /* enable SCLK and SCTL to enable the AMS-POD SPI interface */
                *mcif_control |= 0x03;
                /* set HR bit */
                *mcif_rx_status |= 0x01;

                /* we can also force endian here too.*/
                printf("MCIF_CONTROL value 0x%08x. \n", *((volatile unsigned long*)0xb0400f00));

                /* set up tx and Rx buffer. */
                for (i=0; i<0x1000/4; i++)
                {
                    txbuf[i] = (((i+1)<<16) | (i+2));
                    rxbuf[i] = 0x00;
                }

                /* clear interrupt. */
                *mcif_intr2_cpu_clear = 0x1ff;
                printf("init mcif_intr2_cpu_status %08x\n", *mcif_intr2_cpu_status);

                /* set up TX and RX buffer pointer. */
                *mcif_tx_buf_ptr = ((unsigned long)txbuf) - 0xa0000000ul;
                *mcif_rx_buf_ptr = ((unsigned long)rxbuf) - 0xa0000000ul;
                printf("tx ptr %08x, rx ptr %08x\n", *mcif_tx_buf_ptr, *mcif_rx_buf_ptr);
                *mcif_tx_len = size;
                
                /* start up TX and RX. */
                *mcif_tx_ctrl = 0x118; // play with different IQB values

                /* pull for the done. */
                while ((((temp = *mcif_intr2_cpu_status) & 0x01) != 0x01)); // txdone
//              while ((((temp = *mcif_intr2_cpu_status) & 0x02) != 0x02)); // rxdone
                *((volatile unsigned long*)0xb0400724) |= 0x04000000; // GPIO 58 1.
    
                while ((((temp = *mcif_intr2_cpu_status) & 0x04) != 0x04));
                printf("Transfer done! intr2 status %08x, %08x\n",temp, *mcif_intr2_cpu_status);
                while ((((temp = *mcif_intr2_cpu_status) & 0x03) != 0x03));
                printf("later intr2 status %08x\n",temp);
                while ((((temp = *mcif_intr2_cpu_status) & 0x08) != 0x08));
                printf("last intr2 status %08x\n",temp);

                printf("Rx Data status %08x\n", *mcif_rx_data_status);

                /* clear HR bit. */
                *mcif_rx_status &= 0xfffffffe;
                /* clear interrupt. */
                *mcif_intr2_cpu_clear = 0x1ff;

                /* compare result. */
                temp = 0;
                for (i=0; i<size; i+=4)
                {
                    if (txbuf[i/4] != rxbuf[i/4])
                    {
                        printf("data mismatch offset 0x%04x, tx 0x%08x, rx 0x%08x\n", i, txbuf[i/4],rxbuf[i/4]);
                        temp++;
                        if (temp > 20)
                            break;
                    }
                }

                printf("done!!!\n");
            }
            break;
            #endif
            default:
                printf("Please enter a valid choice\n");
                break;
        }
    }
}


/****************************************************************
* void ProgramDownloadMenu(
*   void )
*
* INPUTS:   none
* OUTPUTS:  none
* RETURNS:  none
* FUNCTION: This function is responsible for providing the user
*           interface when the user wants to programs a newly
*           download executable into Flash or SDRAM.
*
****************************************************************/
static void ProgramDownloadMenu(void)
{
    static unsigned int download_id;
    int quit;


    /* Initialize local variables        */
    quit = 0;
    download_id = 99;

    /*
    * Loop until quit is 1. quit is set to 1 when
    * 0 is entered.
    */

    while (quit == 0)
    {
        printf("        PROGRAM DOWNLOAD MENU \n");
        printf("    0) Return to Main Menu\n");
        printf("    1) Download Program to Flash\n");
        printf("    2) Download program to SDRAM\n");

        download_id = Prompt();
        switch(download_id)
        {
            case  0:
                quit = 1;
                break;
            case  1:
                #ifdef FLASH_TEST
                printf("Use Transfer Drop Down Menu to send file\n");
                printf("Select Xmodem for file transfer\n");
                xmodem_flash_program_download (bcmGetFlashRegHandle());
                #else
                printf("Flash Test not currently supported\n");
                #endif
                break;
            case  2:
                #ifdef FLASH_TEST
                printf("Use Transfer Drop Down Menu to send file\n");
                printf("Select Xmodem for file transfer\n");
                xmodem_sdram_program_download();
                #else
                printf("Flash Test not currently supported\n");
                #endif
                break;
            default:
                printf("Please enter a valid choice\n");
                break;
        }
    }
}
                            
/****************************************************************
* void ReadWriteMenu(
*   void )
*
* INPUTS:   none
* OUTPUTS:  none
* RETURNS:  none
* FUNCTION: This function is responsible for providing the user
*           interface when the user wants to test any memory
*           mapped device, including SDRAM, Flash, ROM, etc.
*
****************************************************************/
static void ReadWriteMenu(void)
{
    char *c_p;
    char str[128];
    unsigned long addr;
    unsigned long val;
    unsigned long j, k;

    printf("\nCommands:\n");
    printf("  dm.[c,s,l] location - read location, add '-b' for block read\n");
    printf("  pm.[c,s,l] location value - write location with value\n");
    printf("  ? - help\n");
    printf("  q - quit\n");

    while (1)
    {
        printf("\n%% ");
        rdstr(str);

        c_p = strtok(str, " {}()\t\n");

        if (c_p != 0)
        {
            if ((strcmp(c_p, "pm.c")==0) || (strcmp(str, "pm")==0))
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&val);
                (*(volatile unsigned char *)(addr)) = val;
            }
            else if (strcmp(c_p, "pm.s") == 0)
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                if (addr & 0x01)
                {
                    printf("Cannot enter an odd address\n");
                    continue;
                }
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&val);
                (*(volatile unsigned short *)(addr)) = val;
            }
            else if (strcmp(c_p, "pm.l") == 0)
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                if (addr & 0x03)
                {
                    printf("Invalid address\n");
                    continue;
                }
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&val);
                (*(volatile unsigned long *)(addr)) = val;
            }
            else if ((strcmp(c_p, "dm.c")==0) || (strcmp(str, "dm")==0))
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                printf("\n%08x: %02x\n", (uint32_t)addr, *(volatile unsigned char *)(addr));
            }
            else if (strcmp(c_p, "dm.c-b")==0)
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                for (j = 0; j < 8; j++)
                {
                    printf("\n%08x: %02x", (uint32_t)addr, *(volatile unsigned char *)(addr));
                    for (k = 0; k < 15; k++)
                    {
                        addr++;
                        printf(" %02x", *(volatile unsigned char *)(addr));
                    }
                    addr++;
                }
            }
            else if (strcmp(c_p, "dm.s") == 0)
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                if (addr & 0x01)
                {
                    printf("Cannot enter an odd address\n");
                    continue;
                }
                printf("\n%08x: %04x\n", (uint32_t)addr, *(volatile unsigned short *)(addr));
            }
            else if (strcmp(c_p, "dm.s-b") == 0)
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                if (addr & 0x01)
                {
                    printf("Cannot enter an odd address\n");
                    continue;
                }
                for (j = 0; j < 8; j++)
                {
                    printf("\n%08x: %04x", (uint32_t)addr, *(volatile unsigned short *)(addr));
                    for (k = 0; k < 7; k++)
                    {
                        addr+=2;
                        printf(" %04x", *(volatile unsigned short *)(addr));
                    }
                    addr+=2;
                }
            }
            else if (strcmp(c_p, "dm.l") == 0)
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                if (addr & 0x03)
                {
                    printf("Invalid address\n");
                    continue;
                }
                printf("\n%08x: %08x\n", (uint32_t)addr, (uint32_t)(*(volatile unsigned long *)(addr)));
            }
            else if (strcmp(c_p, "dm.l-b") == 0)
            {
                c_p = strtok(NULL, " \n\t");
                sscanf(c_p,"%x",(int *)&addr);
                if (addr & 0x03)
                {
                    printf("Invalid address\n");
                    continue;
                }
                for (j = 0; j < 8; j++)
                {
                    printf("\n%08x: %08x", (uint32_t)addr, (uint32_t)(*(volatile unsigned long *)(addr)));
                    for (k = 0; k < 3; k++)
                    {
                        addr+=4;
                        printf(" %08x", (uint32_t)(*(volatile unsigned long *)(addr)));
                    }
                    addr+=4;
                }
            }
            else if ((strcmp(c_p, "?")==0) || (strcmp(str, "help")==0))
            {
                printf("\nCommands:\n");
                printf("  dm.[c,s,l] location - read location\n");
                printf("  pm.[c,s,l] location value - write location with value\n");
                printf("  q - quit\n");
            }
            else if (strcmp(c_p, "q") == 0)
            {
                return;
            }
            else
            {
                printf("\nInvalid Command");
                printf("\nCommands:\n");
                printf("  dm.[c,s,l] location - read location\n");
                printf("  pm.[c,s,l] location value - write location with value\n");
                printf("  q - quit\n");
            }
        }
    }
}

/****************************************************************
* void CsSwapTestMenu(
*   void )
*
* INPUTS:   none
* OUTPUTS:  none
* RETURNS:  none
* FUNCTION: This function is responsible for providing the user
*           interace for CS Swap test  
*
****************************************************************/
void nand_write_all(
    uint32_t block_size
);

#if 0
void CsSwapTestMenu( void )
{
    unsigned int memory_id;
    int quit = 0;
    static unsigned flashTest = 0; /* 0 is for NOR and 1 is for NAND */ 
    /*
     * Loop until quit is 1. quit is set to 1 when
     * 0 is entered.
     */
    while (quit == 0)
    {
        printf("            MEMORY MENU \n");
        printf("    0) Return to Main Menu\n");
        printf("    1) Swap CS address to NAND\n");
        printf("    2) NAND Flash Tests\n");
        printf("    3) Swap CS address to NOR\n");
        printf("    4) NOR Flash Tests\n");


        memory_id = Prompt();
        switch(memory_id)
        {
            case  0:
                quit = 1;
                break;
            case  1:
                flashTest = 1;
                Cs0Disable();
                Cs1EnableForNAND();
                /*nand_write_all(256*1024);*/
                break;
            case  2:
                if (flashTest != 1)
                {
                    printf("NAND test is only valid after selecting(1) \n");
                }
                else
                {
                    NandFlashTestMenu();
                }
                break;
            case  3:
                flashTest = 0;
                Cs1Disable();
                Cs0EnableForNOR();
                break;
            case  4:
                if (flashTest != 0)
                {
                    printf("NOR test is only valid after selecting(3) \n");
                }
                else
                {
                    FlashTestMenu();
                }
                break;
            default:
                printf("Please enter a valid choice\n");
                break;
        }
    }
}
#endif /* (BCHP_CHIP!=7400) */

#if 0
/****************************************************************
* void MemoryProgramBoardParamBlock(
*   void )
*
* INPUTS:   none
* OUTPUTS:  none
* RETURNS:  none
* FUNCTION: This function will accepts data from user, then uses
*           the user input to program the Flash with the board
*           parameter block.
*
****************************************************************/
static void MemoryProgramBoardParamBlock( void )
{
    char str[20];
    unsigned long idx;
    unsigned short board_ser_nbr;
    unsigned short board_rev_nbr;
    int nbr_entry_per_line;
    int board_type_idx;
    const char **board_str;
    BParamBlock bparam_block;
    unsigned int board_tbl_sz;


    /* Allow user to select the board type */
    printf("\n    Board types supported:\n");
    nbr_entry_per_line = 0;
    BParamGetBoardTypeList( &board_str, &board_tbl_sz );
    for( idx = 0; idx < board_tbl_sz; idx++ )
    {
        if( board_str[idx] != NULL )
        {
            printf("%3d=%-7s", idx, board_str[idx]);
            nbr_entry_per_line++;
        }
        if( (nbr_entry_per_line % 6 == 0) && nbr_entry_per_line != 0 )
        {
            // next on display
            printf("\n");
            nbr_entry_per_line = 0;
        }
    }
    printf("\n Select board type from above list (eg: enter 2 for Bcm97115): ");
    rdstr (str);
    board_type_idx = -1;    // make sure what user entered is valid
    sscanf(str, "%d", &board_type_idx);
    if( board_type_idx < 0 || board_type_idx > board_tbl_sz ||
        board_str[board_type_idx] == NULL )
    {
        printf( "\nMemoryProgramBoardParamBlock: Invalid selection, programming abort\n" );
    }
    else
    {
        /* Allow user to enter board serial number */
        printf("\n Enter the board serial id number (eg: 1040): ");
        rdstr (str);
        board_ser_nbr = 0;
        sscanf(str, "%hd", &board_ser_nbr);

        /* Allow user to enter board revision number */
        printf("\n Enter the board revision number (eg: 3): ");
        rdstr (str);
        board_rev_nbr = 0;
        sscanf(str, "%hx", &board_rev_nbr);
        printf("\n");

        if( BParamBuildBoardParamBlock( &bparam_block, board_type_idx,
            board_ser_nbr, board_rev_nbr, eBParamTarget_LE ) == eBParamErrCode_SUCCESS )
        {
            if ( WriteBoardParams(bparam_block.bparam_block, sizeof(bparam_block.bparam_block) ))
            {
                printf("MemoryProgramBoardParamBlock: Programming board parameter block is successful\n");
            }
            else
            {
                printf("MemoryProgramBoardParamBlock: Programming board parameter block is unsuccessful\n");
            }
        }
        else
        {
            printf("MemoryProgramBoardParamBlock: Failed building board paramter block\n");
        }
    }
}


/****************************************************************
* void MemoryDisplayBoardParamBlock(
*   void )
*
* INPUTS:   none
* OUTPUTS:  none
* RETURNS:  none
* FUNCTION: This function is responsible for display the 
*           board parameter block.  The function reads the data
*           from the flash's board paramter block and display
*           the contents to the user.
*
****************************************************************/
static void MemoryDisplayBoardParamBlock( void )
{
    BParamBlock bparam_block;
    unsigned long idx;
    unsigned long board_type;
    unsigned short *board_par;


    if (ReadBoardParams( bparam_block.bparam_block, sizeof(bparam_block.bparam_block) ))
    {
        board_par = bparam_block.bparam_block;
        printf(" Board Parameter Block:\n");
        board_type = (BYTE_SWAP16(board_par[0]) << 16) | BYTE_SWAP16(board_par[1]);
        if( (BYTE_SWAP16(board_par[1]) & 0x00FF) == 0x00 )
        {
            board_type = board_type >> 8;
        }
        printf("  Board Type:     %x\n", board_type );
        printf("  Board Serial #: %d\n", BYTE_SWAP16(board_par[4]));
        printf("  Board Rev. #:   %x\n", BYTE_SWAP16(board_par[5]));
        printf("  MAC Addresses:\n");

        for (idx = 0; idx < NBR_MAC_ADDR; idx++)
        {
            printf("%5d: %04x.%04x.%04x\n", idx,
                BYTE_SWAP16(board_par[6+idx*4]),
                BYTE_SWAP16(board_par[6+idx*4+1]),
                BYTE_SWAP16(board_par[6+idx*4+2]));
        }

        printf("  Checksum:  0x%02x\n\n", (unsigned char)board_par[CHECKSUM_IDX]);
    }
}
#endif

