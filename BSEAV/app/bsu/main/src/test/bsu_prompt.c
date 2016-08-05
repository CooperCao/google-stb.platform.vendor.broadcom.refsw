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
 * Module Description:  Generic prompt for Diagnostics S/W.
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bsu_prompt.h"
#include "bcmuart.h"

#ifndef MIPS_SDE
    #include "uart.h"
#endif

#define UART_STRUCT     UART

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/


/***********************************************************************
 *                      External References
 ***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#if __cplusplus
}
#endif

extern void background(void);
extern int Usb_Serial_Console(void);
extern int Usb_Serial_read( char *buf, int n );
extern int Usb_Serial_write( char *buf, int n );
extern void _writeasm(unsigned long);

/***********************************************************************
 * det_in_char
 *
 * Check if data received on UART A and return data.
 * Otherwise return -1.
 *
 * Input:   char * string
 *
 * Output:  int  -  status
 *
 ***********************************************************************/
char det_in_char(void)
{
    if (*((volatile uint32_t *)(UART_ADR_BASE+UART_SDW_LSR)) & BCHP_UARTA_LSR_DR_MASK)
    {
        return *((volatile uint32_t *)(UART_ADR_BASE+UART_SDW_RBR));
    }

    return (0);
}

char det_in_char_no_background(void)
{
    if (*((volatile uint32_t *)(UART_ADR_BASE+UART_SDW_LSR)) & BCHP_UARTA_LSR_DR_MASK)
    {
        return *((volatile uint32_t *)(UART_ADR_BASE+UART_SDW_RBR));
    }

    return (0);
}

/***********************************************************************
 * rdstr
 *
 * Read input from the default input device
 *
 * Input:   char * string
 *
 * Output:  int  -  status
 *
 ***********************************************************************/
int rdstr(char *s)
{
    char *s_ptr;
    char input_char;

    input_char = 0;
    s_ptr = s;

#ifdef MIPS_SDE
    fflush(stdout);
#else
    printf("\n");
#endif
    while (input_char != '\n')
    {
        /* note that det_in_char already calls OSTimeDly(1) if UCOS_DIAGS and DIAGS_CFE_SUPPORT are both defined.  */
        while ((input_char = det_in_char()) <= 0);
        if ((input_char == '\r') || (input_char == '\n'))
        {
            #if 0
            if (s == s_ptr)
            {
                /* Need at least one character. */
                input_char = 'f'; /* just to make loop to continue */
                printf ("\nInvalid choice\n");
                continue;
            }
            #endif
            break;
        }
        else if (input_char == 0x08)
        {
            s_ptr--;
        }
        else
        {
            *s_ptr++ = input_char;
        }

#ifdef MIPS_SDE
        _writeasm(input_char); /* putchar(input_char); */
#else
        uart_putchar(input_char);
#endif
    }
    *s_ptr = '\0';
    return(strlen(s));
}

#if 0
int rdstr(char *s)
{
    char *s_ptr;
    char input_char;
    int cr=0, lf=0;

    input_char = 0;
    s_ptr = s;

#ifdef MIPS_SDE
    fflush(stdout);
#else
    printf("\n");
#endif
    while (input_char != '\n')
    {
        while ((input_char = det_in_char()) <= 0);
        if (input_char == '\r')
            cr=1;
        if (input_char == '\n')
            lf=1;
        if (cr && lf)
            break;

        if (input_char == 0x08)
            s_ptr--;
        else
            *s_ptr++ = input_char;

        _writeasm(input_char); /* putchar(input_char); */
        #if !defined(LINUX) && !defined(BUILD_SMALL) && !defined(BUILD_SATFE)
            if (Usb_Serial_Console())
                Usb_Serial_write(&input_char, 1);
        #endif
    }
    *s_ptr = '\0';

    return(strlen(s));
}
#endif

/***********************************************************************
 *
 *        GetChar: Wait for user input entry.
 *
 ***********************************************************************/
char GetChar()
{
    signed char input_char;

    input_char = 0;

    /* det_in_char returns 0 if SDE or -1 otherwise for no input */

    /* note that det_in_char already calls OSTimeDly(1) if UCOS_DIAGS and DIAGS_CFE_SUPPORT are both defined.  */
    while (input_char <= 0) {
        input_char = det_in_char();
    }

   return input_char;
}

/***********************************************************************
 *
 *        Prompt: Wait for user input entry.
 *
 ***********************************************************************/
int Prompt()
{
    char choice[10];
    signed char input_char;
    unsigned int command_id;

    input_char = 0;

    printf("\nChoice: ");
#ifdef MIPS_SDE
    fflush(stdout);
#else
    printf("\n");
#endif

    /* det_in_char returns 0 if SDE or -1 otherwise for no input */

    /* note that det_in_char already calls OSTimeDly(1) if UCOS_DIAGS and DIAGS_CFE_SUPPORT are both defined.  */
    while( (input_char = det_in_char()) <= 0 );

    choice[0] = input_char;
    choice[1] = '\0';

    if ((choice[0] == 'y')  ||  (choice[0] == 'Y')  ||
        (choice[0] == 'n')  ||  (choice[0] == 'N'))
        return choice[0];

    sscanf(choice,"%x",&command_id);
    printf("%x\n", command_id);
    return command_id;
}

/***********************************************************************
 *
 *        NoPrompt: Wait for user input entry.
 *
 ***********************************************************************/
int NoPrompt()
{
    char choice[10];
    signed char input_char;
    unsigned int command_id;

    input_char = 0;

    /* det_in_char returns 0 if SDE or -1 otherwise for no input */

    /* note that det_in_char already calls OSTimeDly(1) if UCOS_DIAGS and DIAGS_CFE_SUPPORT are both defined.  */
    while( (input_char = det_in_char()) <= 0 );

    choice[0] = input_char;
    choice[1] = '\0';

    if ((choice[0] == 'y')  ||  (choice[0] == 'Y')  ||
        (choice[0] == 'n')  ||  (choice[0] == 'N'))
        return choice[0];

    sscanf(choice,"%x",&command_id);
    return command_id;
}

/***********************************************************************
 *
 *        PromptChar: Wait for user input entry, while calling background
 *        function, return a character.
 *
 ***********************************************************************/
char PromptChar( void )
{
    char choice[10];
    char input_char;

    input_char = 0;

    printf("\nChoice: ");
#ifdef MIPS_SDE
    fflush(stdout);
#else
    printf("\n");
#endif

    /* note that det_in_char already calls OSTimeDly(1) if UCOS_DIAGS and DIAGS_CFE_SUPPORT are both defined.  */
    while( (input_char = det_in_char()) <= 0 );

    choice[0] = input_char;
    choice[1] = '\0';

    if ((choice[0] == 'y')  ||  (choice[0] == 'Y')  ||
        (choice[0] == 'n')  ||  (choice[0] == 'N'))
        return choice[0];

    printf("\n");
    return input_char;
}

/***********************************************************************
 *
 *        NoPromptChar: Wait for user input entry, while calling background
 *        function, return a character.
 *
 ***********************************************************************/
char NoPromptChar( void )
{
    char choice[10];
    char input_char;

    input_char = 0;

    /* note that det_in_char already calls OSTimeDly(1) if UCOS_DIAGS and DIAGS_CFE_SUPPORT are both defined.  */
    while( (input_char = det_in_char()) <= 0 );

    choice[0] = input_char;
    choice[1] = '\0';

    if ((choice[0] == 'y')  ||  (choice[0] == 'Y')  ||
        (choice[0] == 'n')  ||  (choice[0] == 'N'))
        return choice[0];

    return input_char;
}

/***********************************************************************
 *
 *        BkgdPrompt: Wait for user input entry, while calling background
 *        function, return an integer.
 *
 ***********************************************************************/
int BkgdPrompt( BkgdPromptFunc p_BkgdFunc, void *p_generic )
{
    char choice[10];
    char input_char;
    unsigned int command_id;

    input_char = 0;

    printf("\nChoice: ");
#ifdef MIPS_SDE
    fflush(stdout);
#else
    printf("\n");
#endif

    /* note that det_in_char already calls OSTimeDly(1) if UCOS_DIAGS and DIAGS_CFE_SUPPORT are both defined.  */
    while( (input_char = det_in_char()) <= 0 )
    {
        if( p_BkgdFunc != NULL )
        {
            (*p_BkgdFunc) (p_generic);
        }
    }

    choice[0] = input_char;
    choice[1] = '\0';

    if ((choice[0] == 'y')  ||  (choice[0] == 'Y')  ||
        (choice[0] == 'n')  ||  (choice[0] == 'N'))
        return choice[0];

    sscanf(choice,"%x",&command_id);
    printf("\n");
    return command_id;
}

/***********************************************************************
 *
 *        BkgdPromptChar: Wait for user input entry, while calling background
 *        function, return a character.
 *
 ***********************************************************************/
char BkgdPromptChar( BkgdPromptFunc p_BkgdFunc, void *p_generic )
{
    char choice[10];
    char input_char;

    input_char = 0;

    printf("\nChoice: ");
#ifdef MIPS_SDE
    fflush(stdout);
#else
    printf("\n");
#endif

    /* note that det_in_char already calls OSTimeDly(1) if UCOS_DIAGS and DIAGS_CFE_SUPPORT are both defined.  */
    while( (input_char = det_in_char()) <= 0 )
    {
        if( p_BkgdFunc != NULL )
        {
            (*p_BkgdFunc) (p_generic);
        }
   }

    choice[0] = input_char;
    choice[1] = '\0';

    if ((choice[0] == 'y')  ||  (choice[0] == 'Y')  ||
        (choice[0] == 'n')  ||  (choice[0] == 'N'))
        return choice[0];

    /*sscanf(choice,"%x",&command_id);*/
    printf("\n");
    return input_char;
}
