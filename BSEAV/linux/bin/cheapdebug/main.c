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
 *
 * [File Description:]
 *
 ******************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <sys/utsname.h>
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include <stdlib.h>
#include "bchp_common.h"

#define DBGMSG

#ifdef USE_READLINE
extern HIST_ENTRY **history_list ();
#endif

/**
TODO:
2. support advanced syntax, like "r 4400[2]" and "r 4400-4500"
3. allow infinite looping, and a command to break out of the current loop,
4. specify a loop delay
**/

void printUsage() {
    printf(
    "Usage: cheapdebug [-custom ADDR SIZE] [script]\n"
    "  -custom ADDR SIZE   Use custom memory range\n"
    "\n"
    "Read/Write Commands:\n"
    "  r  [OFFSET]             Read 32 bit value. OFFSET defaults to last OFFSET used.\n"
    "  rb [OFFSET]             Read 8 bit value.\n"
    "  rr FROM TO              Read range of 32 bit values.\n"
    "  d  FROM TO              Dump range of 32 bit values to file \"dump.txt\"\n"
    "  w  {OFFSET|-} [VALUE]   Write 32 bit VALUE to OFFSET. - uses last OFFSET. Defaults to last value read.\n"
    "  wb {OFFSET|-} [VALUE]   Write 8 bit VALUE to OFFSET.\n"
    "Execution Commands:\n"
    "  l  LOOPS*               Set the looping value for the next read command\n"
    "  y  DELAY*               Sleep DELAY milliseconds\n"
    "  s  SCRIPTFILE           Run a script file of commands\n"
    "Current Value Operators:\n"
    "  | VALUE                 'Or' the current value with VALUE.\n"
    "  & VALUE                 'And' the current value with VALUE.\n"
    "  = VALUE                 Set the current value to VALUE.\n"
    "  b [BITNUMBER*] [0|1]    Set bit number BITNUMBER in the \"last value read\" to 0 or 1.\n"
    "        If 0|1 not specified, prints the value of the bit.\n"
    "        If BITNUMBER not specified, prints hex/binary current value\n"
    "Misc:\n"
    "  ?                       Print this help screen\n"
    "  h                       Display command history\n"
    "  q or ^D (EOF)           Quit\n"
    "  #                       A comment line. No action taken\n"
    "\nNotes:\n"
    "All parameters are read as hex, except where explicitly noted with *.\n"
    "All values are displayed in hex.\n"
    "Multiple commands can be entered on a single line with a ; delimiter.\n"
    );
}

static unsigned long curAddress = 0;
static unsigned long curValue = 0;

static char *curValueStr() {
    static char buf[128];
    int i;

    snprintf(buf,sizeof(buf), "%08lx (", curValue);
    for (i=31;i>=0;i--) {
        strncat(buf, curValue & (1<<i) ? "1":"0", 1);
        if (i % 4 == 0 && i)
            strncat(buf, " ",1);
    }
    strncat(buf, ")",1);
    return buf;
}

// filedescriptor for device used to map memory
int fd = -1;

#define REGISTER_BASE   (BCHP_PHYSICAL_OFFSET + (BCHP_REGISTER_START & ~0xFFF))
#define REGISTER_SIZE   (BCHP_REGISTER_END - (BCHP_REGISTER_START & ~0xFFF))

static void get_hardware_address(const char *machine, unsigned long *address, unsigned long *size, unsigned long *address_offset)
{
    *address = REGISTER_BASE;
    *size = REGISTER_SIZE;
    *address_offset = BCHP_REGISTER_START;
}

int main(int argc, char *argv[])
{
    char buffer[256];
    char *cptr = NULL;
    unsigned long base_address, base_address_size, base_address_offset = 0;
    void *base_ptr;
    unsigned long address,i,cnt, value, val32;
    int loop = 0;
    unsigned long delay;
    struct utsname buf;
    FILE *scriptfile = NULL;
    char *script = NULL;
    int custom = 0;

    // determine the chip
    uname(&buf);
    printf("CheapDebug, Machine: %s\n", buf.machine);

    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i], "-custom")) {
            if (argc <= i+2) {
                printUsage();
                return -1;
            }
            base_address = strtoul(argv[++i], NULL, 16);
            base_address_size = strtoul(argv[++i], NULL, 16);
            custom = 1;
        }
        else
            script = argv[i];
    }

    if (custom) {
        printf("Hardware base address: %#lx, size: %#lx\n", base_address, base_address_size);
    }
    else {
        get_hardware_address(buf.machine, &base_address, &base_address_size, &base_address_offset);
    }

    fd = open("/dev/mem",O_RDWR|O_SYNC);/* O_SYNC is for non cached access. */
    if (fd < 0) {
        fprintf(stderr, "Open /dev/mem failed: %d.\nYou probably need to have root access.\n", errno);
        return -1;
    }

    /* mmap page aligned reg base */
    base_ptr = mmap(0, base_address_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, base_address);
    if (base_ptr == (void*)-1) {
        fprintf(stderr, "mmap failed\n");
        return -1;
    }

    /* Now we're ready to start interactive mode */
    if (!script)
        printf("Type ? for help.\n");
    do {
        fflush(NULL);
        if (scriptfile) {
            cptr = fgets(buffer,255,scriptfile);
        }
        else if (script)
            cptr = script;
        else {
#ifdef USE_READLINE
            cptr = readline("CheapDebug>");
            if (cptr) {
                strcpy(buffer, cptr);
                if (*cptr)
                    add_history(cptr);
                free(cptr);
                cptr = buffer;
            }
#else
            printf("CheapDebug>");
            cptr = fgets(buffer,255,stdin);
#endif
        }

        /* EOF condition */
        if (!cptr) {
            if (!scriptfile) {
                printf("\n");
                return 0;
            }
            else {
                fclose(scriptfile);
                scriptfile = NULL;
                continue;
            }
        }

        cptr = strtok(cptr, ";");
        while (cptr) {
            char ch;
            /* skip initial white space */
            cptr += strspn(cptr, " \t");

            /* read first command letter */
            ch = *cptr++;
            switch (ch) {
#ifdef USE_READLINE
            case 'h':
                {
                HIST_ENTRY **list;
                register int i;

                list = history_list ();
                if (list)
                {
                        for (i = 0; list[i]; i++)
                        fprintf (stderr, "%d: %s\r\n", i, list[i]->line);
                }
                }
                break;
#endif

            case '\0':
            case '\n':
            case '#':
                /* allow blank lines and comments in files */
                break;

            case 'b':
                {
                    unsigned long index, value;
                    /* coverity[secure_coding] */
                    int num = sscanf(cptr, "%d %d", &index, &value);
                    switch (num) {
                    case -1:
                        printf("%s\n", curValueStr());
                        break;
                    case 1:
                        printf("bit %d == %d\n", index, (curValue & 1 << index)?1:0);
                        break;
                    default:
                        {
                        int mask = 1 << index;
                        if (value)
                            curValue |= mask;
                        else
                            curValue &= ~mask;
                        printf("value %08X\n", curValue);
                        }
                        break;
                    }
                }
                break;

            case '|':
            case '&':
            case '=':
                {
                    unsigned long value;
                    /* coverity[secure_coding] */
                    if (sscanf(cptr, "%x", &value) == 0) {
                        printf("Missing parameter.\n");
                    }
                    else {
                        switch (ch) {
                        case '|': curValue |= value; break;
                        case '=': curValue = value; break;
                        case '&': curValue &= value; break;
                        }
                        printf("value %08X\n", curValue);
                    }
                }
                break;

            case '?':
                printUsage();
                break;

            case 's':       /* Read script file */
                if (!scriptfile)
                {
                    char script_filename[256];
                    /* coverity[secure_coding] */
                    int n = sscanf(cptr, "%s", script_filename);
                    if (n==1)
                    {
                        scriptfile = fopen(script_filename,"r");
                        if (!scriptfile)
                            printf("ERR: Script file %d open failed, errno=%d\n", script_filename, errno);
                        else
                            printf("Reading from script file %s.\n",script_filename);
                    }
                }
                else
                    printf("Nested scripts are not allowed.\n");
                break;

            case 'q':
                return 0;

            case 'r':
                {
                int byteread = 0;
                unsigned long toaddress;

                ch = *cptr;
                if (ch)
                    cptr++;

                if (loop==0)
                    loop = 1 ;
                if (ch == 'b') {
                    byteread = 1;
                }

                if (ch == 'r') {
                    /* coverity[secure_coding] */
                    if (sscanf(cptr, "%x %x", &address, &toaddress) != 2) {
                        printf("Warning: read command error: expected 2 parameters\n");
                        break;
                    }
                }
                else {
                    /* coverity[secure_coding] */
                    if (sscanf(cptr, "%x", &address) == 0)
                        address = curAddress;
                    toaddress = address;
                }

                {
                    int saveAddress = address;
                    for(;loop;loop--)
                    {
                        while (address <= toaddress) {
                            if (address-base_address_offset >= base_address_size) {
                                fprintf(stderr, "Address %#x out of range. Use should be using an offset from base %#x.\n", address, base_address-base_address_offset);
                                break;
                            }

#define REG_PTR(BASEPTR,ADDR) (&((unsigned char *)(BASEPTR))[ADDR])
                            if (byteread)
                                val32 = *(volatile unsigned char *)REG_PTR(base_ptr,address-base_address_offset);
                            else
                                val32 = *(volatile unsigned *)REG_PTR(base_ptr,address-base_address_offset);
                            curValue = val32;
                            curAddress = address;
                            if (byteread)
                                printf("address %08X, value %02X\n", address, val32);
                            else
                                printf("address %08X, value %08X\n", address, val32);
                            address += 4;
                        }
                        address = saveAddress;
                    }
                }
                }
                break;

            case 'w':
                {
                int bytewrite = 0;

                ch = *cptr;
                if (ch)
                    cptr++;
                if (ch == 'b')
                    bytewrite = 1;

                cptr += strspn(cptr, " \t");
                if (cptr[0] == '-') {
                    int n;
                    address = curAddress;
                    /* coverity[secure_coding] */
                    n = sscanf(&cptr[1], "%x", &value);
                    if (n < 1)
                        value = curValue;
                }
                else {
                    /* coverity[secure_coding] */
                    int n = sscanf(cptr, "%x %x", &address, &value);
                    if (n < 2)
                        value = curValue;
                    if (n < 1)
                        address = curAddress;
                }

                if (address-base_address_offset >= base_address_size) {
                    printf("Address out of range. Use should be using an offset from the base.\n");
                    break;
                }

                if (bytewrite)
                    *(volatile unsigned char *)REG_PTR(base_ptr,address-base_address_offset) = value;
                else
                    *(volatile unsigned *)REG_PTR(base_ptr,address-base_address_offset) = value;

                printf("write address %08X with %08X\n", address, value);
                curValue = value;
                curAddress = address;
                }
                break;

            case 'd':
                {
                unsigned long eaddress;
                int n;

                if (*cptr)
                    ch = *cptr++;

                /* coverity[secure_coding] */
                n = sscanf(cptr, "%x %x", &address,&eaddress);
                if (n != 2) {
                    printf("Warning: read command error - parameter error\n");
                } else
                {
                    FILE *tf = fopen("dump.txt","wb+");
                    if (!tf)
                    {
                        printf("could not open dump.txt\n");
                        break;
                    }
                    cnt = 0;
                    for (i = address-base_address_offset; i <= eaddress-base_address_offset; i += 4)
                    {
                        val32 = *(volatile unsigned *)REG_PTR(base_ptr,i);
                        fprintf(tf,"\n0x%08X(0x%08x):  %08X", i, ((i - 0x4000)/4),val32);
                    }
                    fclose(tf);
                }
                }
                break;

            case 'l':
                {
                int n;
                cptr++;
                /* coverity[secure_coding] */
                n = sscanf(cptr, "%d", &value);
                loop = (n != 1)? 1 : (value) ? value : 1 ;
                printf("Loop set to %d times\n", loop) ;
                }
                break;

            case 'y':
                {
                int n;

                cptr++;
                /* coverity[secure_coding] */
                n = sscanf(cptr, "%d", &delay);
                if (n != 1)
                {
                    printf("Warning: delay command error - parameter error\n");
                }
                else
                {
                    printf("delay %d ms\n", delay);
                    usleep(delay*1000);
                }
                }
                break;

            default:
                printf("Warning: Unknown command: '%c'.\n\n", ch);
                printUsage();
                break;
            }

            cptr = strtok(NULL, ";");
        } /* strtok while */

        if (script)
            break;
    } while (1);
    return 0;
}

