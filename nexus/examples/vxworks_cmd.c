/***************************************************************************
 *     (c)2005-2008 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 */

#include <stdio.h>
#include <string.h>
#include <taskLib.h>

/*
** An application is started in the VxWorks environment by entering (typing) the function name in the target shell.
** If no parameters are passed (in parenthesis) then all arguments to the function are provided as 0 (or null).
** Providing a list of arguments (for argv in main) is hard to do.  This provides a wrapper function that parses
** the command line (parameters passed) into space separated arguments and passes those arguments to main.
*/

extern int main(int argc, char **argv);

int go(char *cmd)
{
	int argc = 0;
	char *argv[100];
	char *s;

	/* This assumes that the first param in the script will be the app name (standard) */

    /* If they don't include any options or if they start with just the options listed,
    ** apply a dummy 'program name' option to the front of the argument list.
    */
    if (!cmd || *cmd == '-') {
        argv[argc++] = "go";
    } else {
        s = strtok(cmd, " ");
        while (s) {
            printf("argv[%d] = %s%s\n", argc, s, argc?"":" <-- program name");
            argv[argc++] = s;
            s = strtok(NULL, " ");
        }
    }
    
    /* Need this to prevent a crash -- we always get an argc and argv, right? */
    if (!argc) argv[argc++] = "go";
    
    /* VxWorks starts the default task off with a high priority.
    ** Need apps to run at priority less than the other maintenence tasks.
    */
    taskPrioritySet(taskIdSelf(),105);

	return main(argc, argv);
}

#define DEFAULT    123456789L  /* initial seed, use 0 < DEFAULT < MODULUS  */
#define MODULUS    2147483647L /* DON'T CHANGE THIS VALUE                  */
#define MULTIPLIER 48271L      /* DON'T CHANGE THIS VALUE                  */
      
/* The standard VxWorks "rand" function returns a number between 0-32767 -- we need one a bit larger. */
int rand(void)
{
    static int seed = DEFAULT;
    const long Q = MODULUS / MULTIPLIER;
    const long R = MODULUS % MULTIPLIER;
    long t;

    t = MULTIPLIER * (seed % Q) - R * (seed / Q);
    if (t > 0) 
        seed = t;
    else 
        seed = -t;
    return seed;
}


