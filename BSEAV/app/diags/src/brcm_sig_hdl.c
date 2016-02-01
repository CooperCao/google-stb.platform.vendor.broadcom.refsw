/******************************************************************************
 *    (c)2008-2010 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/

#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <dirent.h>
#include <dlfcn.h>


#include "ucontext.h"
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>


typedef unsigned int size_t;
#define abs(s) ((s) < 0 ? -(s) : (s))


/* rule#1 - try to keep a line inside 80 characters
 * rule#2 - try construct one line at a time
 */
#define MAX_OUTPUT_BUFFER 128
static char outputBuffer[MAX_OUTPUT_BUFFER];
static const char HexTable[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};
/* ul2hex -- converts unsigned long (32 bits) into 8 digit hex-decimal string
 *
 * Parameters:
 *  val [in] 32 bits unsigned long value to be converted into human readable
 *  hex [out] buffer to hold text converted format
 */
static void ul2hex(unsigned long val, char *hex)
{
    if (NULL == hex) return;

    hex[7] = HexTable[val&0xF];
    val >>= 4;
    hex[6] = HexTable[val&0xF];
    val >>= 4;
    hex[5] = HexTable[val&0xF];
    val >>= 4;
    hex[4] = HexTable[val&0xF];
    val >>= 4;
    hex[3] = HexTable[val&0xF];
    val >>= 4;
    hex[2] = HexTable[val&0xF];
    val >>= 4;
    hex[1] = HexTable[val&0xF];
    val >>= 4;
    hex[0] = HexTable[val&0xF];
}

/* int sigbacktrace_mips32(void **buffer, int size,struct my_ucontext *uc); */
int sigbacktrace_mips32(void **buffer, int size,struct ucontext *uc)
{
    unsigned long *addr, *ra, *sp, *pc;
    size_t ra_offset, stack_size;
    int depth;
    char *p;

    if(!size)
        return 0;
    if(!buffer || size < 0 || !uc)
        return -EINVAL;
    /* get current $pc,$ra and $sp */

	if ( uc->uc_mcontext.sc_regs[31] & 0x1 ) {
		/* pc is an odd address ! */
    	memset(outputBuffer, 0, sizeof(outputBuffer));
	    p = outputBuffer;
	    strcpy(p, __FUNCTION__); p += strlen(p);
	    strcpy(p, " ra IS AN ODD ADDRESS - FIX TO: "); p += 32; /*  4 == strlen(" pc "); */
    	ul2hex(uc->uc_mcontext.sc_regs[31] & 0xFFFFFFFC , p); p += 8;
	    *p = '\n'; ++p;
	    write(1, outputBuffer, p - outputBuffer);

		ra = (unsigned long*)(unsigned long) (uc->uc_mcontext.sc_regs[31] & 0xFFFFFFFC);
		
	}
	else     
    	ra = (unsigned long*)(unsigned long)uc->uc_mcontext.sc_regs[31];

	if ( uc->uc_mcontext.sc_pc & 0x1 ) {
		/* pc is an odd address ! */
    	memset(outputBuffer, 0, sizeof(outputBuffer));
	    p = outputBuffer;
	    strcpy(p, __FUNCTION__); p += strlen(p);
	    strcpy(p, " pc IS AN ODD ADDRESS - FIX TO: "); p += 32; /*  4 == strlen(" pc "); */
    	ul2hex(uc->uc_mcontext.sc_pc & 0xFFFFFFFC , p); p += 8;
	    *p = '\n'; ++p;
	    write(1, outputBuffer, p - outputBuffer);

	    pc = (unsigned long*)(unsigned long)(uc->uc_mcontext.sc_pc & 0xFFFFFFFC);
	}
	else 
	    pc = (unsigned long*)(unsigned long)uc->uc_mcontext.sc_pc;

	if ( uc->uc_mcontext.sc_regs[29] & 0x1 ) {
		/* sp is an odd address ! */
    	memset(outputBuffer, 0, sizeof(outputBuffer));
	    p = outputBuffer;
	    strcpy(p, __FUNCTION__); p += strlen(p);
	    strcpy(p, " sp IS AN ODD ADDRESS - FIX TO: "); p += 32; /*  4 == strlen(" pc "); */
    	ul2hex(uc->uc_mcontext.sc_regs[29] & 0xFFFFFFFC , p); p += 8;
	    *p = '\n'; ++p;
	    write(1, outputBuffer, p - outputBuffer);

	    sp = (unsigned long*)(unsigned long)(uc->uc_mcontext.sc_regs[29] & 0xFFFFFFFC);
	}
	else 
	    sp = (unsigned long*)(unsigned long)uc->uc_mcontext.sc_regs[29];

    buffer[0] = pc;
    if(size == 1)
    {
    	memset(outputBuffer, 0, sizeof(outputBuffer));
	    p = outputBuffer;
	    strcpy(p, __FUNCTION__); p += strlen(p);
	    strcpy(p, " size is 1, early return"); p += 24; /*  4 == strlen(" pc "); */
	    *p = '\n'; ++p;
	    write(1, outputBuffer, p - outputBuffer);
        return 1;
    }

    memset(outputBuffer, 0, sizeof(outputBuffer));
    p = outputBuffer;
    strcpy(p, __FUNCTION__); p += strlen(p);
    strcpy(p, " pc "); p += 4; /*  4 == strlen(" pc "); */
    ul2hex(uc->uc_mcontext.sc_pc, p); p += 8;
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);

    /* scanning to find the size of the current stack-frame */
    ra_offset = stack_size = 0;
    for(addr = pc; !ra_offset || !stack_size; --addr)
    {
        switch(*addr & 0xffff0000)
        {
            case 0x27bd0000:  /* 0x27bd 0000 == addiu sp,sp, 0  - so *addr & 0xffff gives you stack size */
                stack_size = abs((short)(*addr & 0xffff));
                break;
            case 0xafbf0000: /* 0xafbf 0000 == sw ra,0(sp)  - ie it is saving registers on local stack */
                ra_offset = (short)(*addr & 0xffff);
                break;
            case 0x3c1c0000:
                goto __out_of_loop;

            default:
                break;
        }
    }
__out_of_loop:
    if(ra_offset)
    {
        ra = *(unsigned long**)((unsigned long)sp + ra_offset);

	    memset(outputBuffer, 0, sizeof(outputBuffer));
	    p = outputBuffer;
	    strcpy(p, __FUNCTION__); p += strlen(p);
	    strcpy(p, " ra_offset  = "); p += 14; /* 14 == strlen(" ra_offset  = "); */
	    ul2hex((unsigned long)ra_offset, p); p += 8;
	    *p = '\n'; ++p;
	    write(1, outputBuffer, p - outputBuffer);
    }
    if(stack_size)
    {
        sp = (unsigned long*)((unsigned long)sp + stack_size);

	    memset(outputBuffer, 0, sizeof(outputBuffer));
	    p = outputBuffer;
	    strcpy(p, __FUNCTION__); p += strlen(p);
	    strcpy(p, " stack_size = "); p += 14; /* 14 == strlen(" stack_size = "); */
	    ul2hex((unsigned long)stack_size, p); p += 8;
	    *p = '\n'; ++p;
	    write(1, outputBuffer, p - outputBuffer);
    }

    /* printf("%s ra %p, sp %p\n", __FUNCTION__, ra, sp); */
    memset(outputBuffer, 0, sizeof(outputBuffer));
    p = outputBuffer;
    strcpy(p, __FUNCTION__); p += strlen(p);
    strcpy(p, " ra "); p += 4; /* 4 == strlen(" ra "); */
    ul2hex((unsigned long)ra, p); p += 8;
    strcpy(p, ", sp "); p += 5; /* 5 == strlen(", sp "); */
    ul2hex((unsigned long)sp, p); p += 8;
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);
    /* printf("\t$0  : %08X %08X %08X %08X\n" */
    /*        "\t$4  : %08X %08X %08X %08X\n", r0, r1, r2, r3, r4, r5, r6, r7); */
    memset(outputBuffer, 0, sizeof(outputBuffer));
    p = outputBuffer;
    strcpy(p,   "\t$0       : "); p += 12;         /* 12 == strlen("\t$0  : ") */
    ul2hex(uc->uc_mcontext.sc_regs[0], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[1], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[2], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[3], p); p += 8;
    strcpy(p, "\n\t$4       : "); p += 13;         /* 13 == strlen("\n\t$4     : ") */
    ul2hex(uc->uc_mcontext.sc_regs[4], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[5], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[6], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[7], p); p += 8;
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);

    /* printf("\t$8  : %08X %08X %08X %08X\n"
     *             "\t$12 : %08X %08X %08X %08X\n", ...);
     *                  okay to skip memset(outputBuffer, 0, sizeof(outputBuffer)); */
    p = outputBuffer;
    strcpy(p,   "\t$8       : "); p += 12;         /* 12 == strlen("\t$8  : ") */
    ul2hex(uc->uc_mcontext.sc_regs[8], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[9], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[10], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[11], p); p += 8;
    strcpy(p, "\n\t$12      : "); p += 13;         /* 13 == strlen("\n\t$12    : ") */
    ul2hex(uc->uc_mcontext.sc_regs[12], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[13], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[14], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[15], p); p += 8;
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);

    /* printf("\t$16 : %08X %08X %08X %08X\n"
              "\t$20 : %08X %08X %08X %08X\n", ...);
               okay to skip memset(outputBuffer, 0, sizeof(outputBuffer));  */
    p = outputBuffer;
    strcpy(p,   "\t$16      : "); p += 12;         /* 12 == strlen("\t$16   : ") */
    ul2hex(uc->uc_mcontext.sc_regs[16], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[17], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[18], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[19], p); p += 8;
    strcpy(p, "\n\t$20      : "); p += 13;         /* 13 == strlen("\n\t$20    : ") */
    ul2hex(uc->uc_mcontext.sc_regs[20], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[21], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[22], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[23], p); p += 8;
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);
    /* printf("\t$24 : %08X %08X %08X %08X\n"
              "\t$28 : %08X %08X %08X %08X\n", ...);
               okay to skip memset(outputBuffer, 0, sizeof(outputBuffer));  */
    p = outputBuffer;
    strcpy(p,   "\t$24      : "); p += 12;          /* 12 == strlen(  "\t$24       : ") */
    ul2hex(uc->uc_mcontext.sc_regs[24], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[25], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[26], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[27], p); p += 8;
    strcpy(p, "\n\t$28      : "); p += 13;          /* 13 == strlen("\n\t$28      : ") */
    ul2hex(uc->uc_mcontext.sc_regs[28], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[29], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[30], p); p += 8;
    *p = ' '; ++p;
    ul2hex(uc->uc_mcontext.sc_regs[31], p); p += 8;
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);

    /* printf("\tStatus: %p\n"
              "\tCause : %p\n"
              "\tBadVaddr : %p\n",
            uc->uc_mcontext.sc_status, uc->uc_mcontext.sc_hi1,  uc->uc_mcontext.sc_lo1);
    */
    memset(outputBuffer, 0, sizeof(outputBuffer));
    p = outputBuffer;
    strcpy(p,   "\tStatus   : "); p += 12;         /* 12 == strlen("\tStatus : ") */
    ul2hex(uc->uc_mcontext.sc_status, p); p += 8;
    strcpy(p, "\n\tCause    : "); p += 13;         /* 13 == strlen("\n\tCause  : ") */
    ul2hex(uc->uc_mcontext.sc_hi1, p); p += 8;
    strcpy(p, "\n\tBadVaddr : "); p += 13;         /* 13 == strlen("\n\tBadVaddr : ") */
    ul2hex(uc->uc_mcontext.sc_lo1, p); p += 8;
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);

    /* repeat backward scanning */
    for(depth = 1; depth < size && ra; ++depth)
    {
        buffer[depth] = ra;
        ra_offset = stack_size = 0;
        for(addr = ra;!ra_offset || !stack_size; --addr)
        {
            switch(*addr & 0xffff0000)
            {
                case 0x27bd0000: /* 0x27bd 0000 == addiu sp,sp, 0  - so *addr & 0xffff gives you stack size */
                stack_size = abs((short)(*addr & 0xffff));
                break;
                case 0xafbf0000: /* 0xafbf 0000 == sw ra,0(sp)  - ie it is saving registers on local stack */
                ra_offset = (short)(*addr & 0xffff);
                break;
                case 0x3c1c0000:  /* in assembly = lui  gp,0x0 */
					/* Added because sigbacktrace_mips32 often returns and doesn't print rest of debug */
    				memset(outputBuffer, 0, sizeof(outputBuffer));
				    p = outputBuffer;
					strcpy(p, __FUNCTION__); p += strlen(p);
				    strcpy(p, "lui gp depth="); p += 13; /*  p=7 because spacedepth= */
				    ul2hex(depth+1, p); p += 8;
				    *p = '\n'; ++p;
				    write(1, outputBuffer, p - outputBuffer);
                return depth+1;

                default:
                break;
            }
        }
        ra = *(unsigned long**)((unsigned long)sp + ra_offset);
        sp = (unsigned long*)((unsigned long)sp + stack_size);
    }
	/* Added because sigbacktrace_mips32 often returns and doesn't print rest of debug */
    strcpy(p, __FUNCTION__); p += strlen(p);
    strcpy(p, " depth="); p += 7; /*  p=7 because spacedepth= */
    ul2hex(depth, p); p += 8;
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);

    return depth;
}


static char ** card_backtrace_symbols ( void *const *array, int size)
{
    int cnt;
    size_t total = 0;
    char **result;
    /* Assume the worst for the width of an address.  */
    #define WORD_WIDTH 16

    if( size < 1 )
    {
        char *p;

        /* printf("[%s] No stacked pointer(%d)\n", __FUNCTION__, size); */
        memset(outputBuffer, 0, sizeof(outputBuffer));
        p = outputBuffer;
        strcpy(p, "[card_backtrace_symbols] No stacked pointer\n");
        p += strlen(p);
        write(1, outputBuffer, p - outputBuffer);
    }

    /* We can compute the text size needed for the symbols since we print
       them all as "[+0x<addr>]".  */
    total = size * (WORD_WIDTH + 6);

    /* Allocate memory for the result.  */
    result = (char **)malloc (size * sizeof (char *) + total);
    if (result != NULL)
    {
        char *last = (char *) (result + size);

        for (cnt = 0; cnt < size; ++cnt)
        {
            result[cnt] = last;
            /* last +=
               1 + sprintf (last, "[+0x%08x]", (unsigned int)array[cnt]);
            */
            strcpy(last, "[+0x"); last += 4; /* 4 == strlen("[+0x") */
            ul2hex((unsigned long)array[cnt], last); last += 8;
            *last = ']'; ++last;
            *last = '\0'; ++last;
        }
    }

    return result;
}

#define SIZE 20
static void sig_handler_with_arg(int signal,siginfo_t *sig_info,void *context)
{
    void *buf[SIZE];
    int d, i;
    char **syms;
    pid_t Pid;
    char SysCall[100];
    Dl_info info;
    char *p;
    struct ucontext *pUc = (struct ucontext*) context;

#ifdef CREATE_TRACE_LOG_FILE
    int fd;
    FILE *fp = NULL;
    int j;
#endif

	/* print pid early because sometmes don't get to pid printing part */
	{
        Pid=getpid();
        /* printf("PID of this process is %d\n", Pid); */
        memset(outputBuffer, 0, sizeof(outputBuffer));
        p = outputBuffer;
        strcpy(p, "PID of this process is 0x"); p += strlen(p);
        ul2hex(Pid, p); p += 8; /* pid */
        *p = '\n'; ++p;
        write(1, outputBuffer, p - outputBuffer);
	}

    memset(outputBuffer, 0, sizeof(outputBuffer));
    p = outputBuffer;
    strcpy(p, __FUNCTION__); p += strlen(p);
    strcpy(p, " sig "); p += 5; /* 5 == strlen(" sig ") */
    ul2hex(signal, p); p += 8;


    if (NULL == pUc)
    {
        /* printf("%s sig %d no ctxt\n", __FUNCTION__, signal); */
        strcpy(p, "no ctxt"); p += 7; /* 7 == strlen("no ctxt") */
        /* This should never happen, but if it does this signal handler will
         * crash again after printing out "no ctxt". */
    }
    else
    {
        /* printf("%s sig %d, pc %08x, ra %08x\n", __FUNCTION__, signal,
         *        (unsigned int) pUc->uc_mcontext.sc_pc,
         *        (unsigned int) pUc->uc_mcontext.sc_regs[31]);
         */
        strcpy(p, ", pc "); p += 5; /* 5 == strlen(", pc ") */
        ul2hex(pUc->uc_mcontext.sc_pc, p); p += 8; /* pc */
        strcpy(p, ", ra "); p += 5; /* 5 == strlen(", ra ") */
        ul2hex(pUc->uc_mcontext.sc_regs[31], p); p += 8; /* ra */
    }
    *p = '\n'; ++p;
    write(1, outputBuffer, p - outputBuffer);

    /* Check if either program counter or return address are ODD  */
    if ( pUc->uc_mcontext.sc_pc & 0x1 || pUc->uc_mcontext.sc_regs[31] & 0x1 ) {
        /* pc and ra should NEVER be ODD, dump stack contents so we can check */
        unsigned long *stackAddr;

        /* Show the system map on the screen */
        sprintf(SysCall, "cat /proc/%d/maps", Pid);
        system(SysCall);

        /* Put line feed space in between map info and stack info */
        memset(outputBuffer, 0, sizeof(outputBuffer));
        p = outputBuffer;
        *p = '\n'; ++p;
        write(1, outputBuffer, p - outputBuffer);

        stackAddr = (unsigned long*)(unsigned long)pUc->uc_mcontext.sc_regs[29] - 32;
        for ( i=0 ; i < 256/16; i++ ) {
            memset(outputBuffer, 0, sizeof(outputBuffer));
            p = outputBuffer;
            strcpy(p, "\t"); p += 1; /* 7 == strlen("\t$0  : ") */
            ul2hex( (unsigned long)stackAddr, p); p += 8;
            strcpy(p, "  : "); p += 4; /* 7 == strlen("\t$0  : ") */

            ul2hex( *stackAddr, p); p += 8;
            *p = ' '; ++p;
            stackAddr++;
            ul2hex( *stackAddr, p); p += 8;
            *p = ' '; ++p;
            stackAddr++;
            ul2hex( *stackAddr, p); p += 8;
            *p = ' '; ++p;
            stackAddr++;
            ul2hex( *stackAddr, p); p += 8;
            stackAddr++;
            *p = '\n'; ++p;
            write(1, outputBuffer, p - outputBuffer);
        }
        memset(outputBuffer, 0, sizeof(outputBuffer));
        p = outputBuffer;
        *p = '\n'; ++p;
        write(1, outputBuffer, p - outputBuffer);
    }

    if( (signal==SIGINT)
        || (signal==SIGSEGV)
        || (signal==SIGTERM)
        || (signal==SIGFPE)
        || (signal==SIGBUS)
    )
    {
        d = sigbacktrace_mips32(buf, SIZE, (struct ucontext*)context);
        if( d < 1 )
        {
            /* printf("[%s] Invalid depth(%d)\n", __FUNCTION__, d); */
            memset(outputBuffer, 0, sizeof(outputBuffer));
            p = outputBuffer;
            strcpy(p, "[sig_handler_with_arg] Invalid depth("); p += strlen(p);
            switch (d) {
            case 0:
                strcpy(p, "0)\n"); p += 3; /* 3 == strlen("0)\n") */
                break;
            case -EINVAL:
                strcpy(p, "EINVAL)\n"); p += 8; /* 8 == strlen("EINVAL)\n") */
                break;
            default:
                strcpy(p, "err)\n"); p += 5; /* 5 == strlen("err)\n") */
                break;
            }
            write(1, outputBuffer, p - outputBuffer);
        }
        else
        {
            syms = card_backtrace_symbols(buf, d);

#ifdef CREATE_TRACE_LOG_FILE
            /* fp = fopen("/mnt/nvram/bt.txt", "w"); */
            /* MPLCore_Assert(fp) ; */
            fd = open("/mnt/nvram/bt.txt", O_WRONLY|O_CREAT, 00440);
            if(fd == 0)
            {
                printf("Can't open fd\n");
                exit(-1);
            }
#endif

            /* printf("-------------------------------------------------\n"); */
            memset(outputBuffer, 0, sizeof(outputBuffer));
            p = outputBuffer;
            strcpy(p, "-------------------------------------------------\n");
            p += strlen(p);
            write(1, outputBuffer, p - outputBuffer);

            for(i = 0;i < d;i++)
            {
                memset(outputBuffer, 0, sizeof(outputBuffer));
                p = outputBuffer;
                if(!dladdr(buf[i], &info))
                {
                    /* printf("%s -\n", syms[i]); */
                    strcpy(p, syms[i]); p += strlen(p);
                    *p = ' '; ++p;
                    *p = '-'; ++p;
                    *p = '\n'; ++p;
                }
                else
                {
                    /* printf("%s - %s:%s [0x%08x]\n", syms[i],
                     *        info.dli_fname, info.dli_sname,
                     *        (char *)buf[i]-(char *)info.dli_fbase);
                     */
                    strcpy(p, syms[i]); p += strlen(p);

                    *p = ' '; ++p;
                    *p = '-'; ++p;
                    *p = ' '; ++p;
                    strcpy(p, info.dli_fname); p += strlen(p);
                    *p = ':'; ++p;
                    strcpy(p, info.dli_sname); p += strlen(p);
                    strcpy(p, " [0x"); p += 4; /* 4 == strlen(" [0x") */
                    ul2hex((char *)buf[i]-(char *)info.dli_fbase, p); p += 8;
                    *p = ']'; ++p;
                    *p = '\n'; ++p;
                }
                write(1, outputBuffer, p - outputBuffer);

#ifdef CREATE_TRACE_LOG_FILE
                int n=strlen(syms[i]);
                for( j= 4;j < n;j++)
                {
                    if((syms[i])[j]==']')
                    {
                        break;
                    }
                    /* fputc((syms[i])[j], fp); */
                    outputBuffer[0] = syms[i][j];
                    write(fd, outputBuffer, 1);
                }
                /* fprintf(fp, " - %s:%s [0x%08X]\n", info.dli_fname,
                   info.dli_sname, buf[i]-info.dli_fbase);
                */
                memset(outputBuffer, 0, sizeof(outputBuffer));
                p = outputBuffer;
                *p = ' '; ++p;
                *p = '-'; ++p;
                *p = ' '; ++p;
                strcpy(p, info.dli_fname); p += strlen(p);
                *p = ':'; ++p;
                strcpy(p, info.dli_sname); p += strlen(p);
                p += strlen(p);
                strcpy(p, " [0x"); p += 4; /* 4 == strlen(p); */
                ul2hex((char *)buf[i]-(char *)info.dli_fbase, p); p += 8;
                *p = ']'; ++p;
                *p = '\n'; ++p;
                write(fd, outputBuffer, p - outputBuffer);
#endif
            }
#ifdef CREATE_TRACE_LOG_FILE
            close(fd);
#endif
            free(syms);
        }
        Pid=getpid();
        /* printf("PID of this process is %d\n", Pid); */
        memset(outputBuffer, 0, sizeof(outputBuffer));
        p = outputBuffer;
        strcpy(p, "PID of this process is 0x"); p += strlen(p);
        ul2hex(Pid, p); p += 8; /* pid */
        *p = '\n'; ++p;
        write(1, outputBuffer, p - outputBuffer);

        /* Show the system map on the screen */
        sprintf(SysCall, "cat /proc/%d/maps", Pid);
        system(SysCall);

#ifdef CREATE_TRACE_LOG_FILE
        /* Copy the map file to nvram */
        sprintf(SysCall, "cp /proc/%d/maps /mnt/nvram/maps", Pid);
        system(SysCall);

        /* Copy the terminal log file to nvram (will only exist if using the redir tool to redirect stdout/stderr. But ok if this fails, no big */
        system("sync;cp /mnt/etc/log/log.txt /mnt/nvram/log.txt");

        /* See if a debug dump server exists */
        fp=fopen("/mnt/nvram/crashserver.txt","r");
        if(fp)
        {
            /* Server config exists, copy files to the server */
            char UniqueName[100];
            char Login[100], Pass[100], IP[16], Dir[100];
            time_t CurTime;

            /* Get server IP, login, pass */
            fscanf(fp, "%s\n", IP);
            fscanf(fp, "%s\n", Login);
            fscanf(fp, "%s\n", Pass);
            fscanf(fp, "%s\n", Dir);
            fscanf(fp, "%s\n", UniqueName);
            fclose(fp);

            /* generate unique name */
            CurTime=time(NULL);
            sprintf(&UniqueName[strlen(UniqueName)], "-%d", (int)CurTime);

            /* Put the files */
            fprintf(stderr, "Attempting to copy log files to server %s with prefix %s/%s\n", IP, Dir, UniqueName);

            sprintf(SysCall, "ftpput -u %s -p %s %s %s/%s_bt.txt /mnt/nvram/bt.txt\n", Login, Pass, IP, Dir, UniqueName);
            system(SysCall);

            sprintf(SysCall, "ftpput -u %s -p %s %s %s/%s_maps /mnt/nvram/maps\n", Login, Pass, IP, Dir, UniqueName);
            system(SysCall);

            sprintf(SysCall, "ftpput -u %s -p %s %s %s/%s_log.txt /mnt/nvram/log.txt\n", Login, Pass, IP, Dir, UniqueName);
            system(SysCall);
        }
#endif
        fprintf(stderr, "%s [line: %d] end\n",__FUNCTION__,__LINE__);

        exit(-1);
    }
}

void installSignalHandler( unsigned int pid )
{

    struct sigaction sig_act;
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_sigaction=sig_handler_with_arg;
    sig_act.sa_flags=SA_SIGINFO;
    sigaction(SIGINT,&sig_act,NULL);
    sigaction(SIGSEGV,&sig_act,NULL);
    sigaction(SIGTERM,&sig_act,NULL);
    sigaction(SIGFPE,&sig_act,NULL);

/*    sigaction(SIGCHLD,&sig_act,NULL); used for testing */

    sigaction(SIGBUS,&sig_act,NULL);
}

