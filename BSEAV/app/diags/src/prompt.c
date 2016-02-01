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
 * Module Description:  Generic prompt for Diagnostics S/W.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/***********************************************************************
 *
 *        Prompt: Display prompt, wait for user input entry, return an
 *        integer.
 *
 ***********************************************************************/
int Prompt(void)
{
    char buf[256];
    printf("\nChoice: ");
    fgets(buf, 256, stdin);
    return atoi(buf);
}

/***********************************************************************
 *
 *        NoPrompt: Wait for user input entry, return an integer.
 *
 ***********************************************************************/
int NoPrompt(void)
{
    char buf[256];
    fgets(buf, 256, stdin);
    return atoi(buf);
}

/***********************************************************************
 *
 *        NoPromptChar: Wait for user input entry in hex, return an
 *        integer.
 *
 ***********************************************************************/
int NoPromptHex(void)
{
    char buf[256];
    fgets(buf, 256, stdin);
    return (strtol(buf, NULL, 0));
}

/***********************************************************************
 *
 *        PromptChar: Display prompt, wait for user input entry,
 *        return a character.
 *
 ***********************************************************************/
char PromptChar( void )
{
    char buf[256];
    printf("\nChoice: ");
    fgets(buf, 256, stdin);
    return buf[0];
}

/***********************************************************************
 *
 *        NoPromptChar: Wait for user input entry, return a character.
 *
 ***********************************************************************/
char NoPromptChar( void )
{
    char buf[256];
    fgets(buf, 256, stdin);
    return buf[0];
}
