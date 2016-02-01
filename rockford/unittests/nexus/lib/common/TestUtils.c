/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 * Module Description: Generic test Utils source for test apps
 *
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "TestUtils.h"

#ifdef HAS_NEXUS /* TBD: Need to find the appropriate define */
	#include "nexus_platform.h"
	BDBG_MODULE(TestUtils);
#else
	#define	BDBG_ERR(A) 	\
		do {				\
			printf A;		\
			printf("\n");	\
		} while (0)

	#define BDBG_WRN	BDBG_ERR
	#define BDBG_LOG	BDBG_ERR
#endif

int options_parse(int argc, char **argv, CLArgs *args, int len)
{
    int i, j, found;

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            found = 0;
            for (j = 0; j < len; j++)
            {
                if (strcmp(&argv[i][1], args[j].tag) == 0)
                {
                    found = 1;
                    if (args[j].set != 0)
                    {
                        BDBG_ERR(("Duplicate option %s", argv[i]));
                    }
                    if (argv[i+1] != NULL)
                    {
                        if (args[j].val_required != 0)
                        {
                            ++i;
                            if (args[j].set == 0)
                            {
                                args[j].val = argv[i];
                            }
                        }
                    }
                    args[j].set = 1;
                    break;
                }
            }
            if (found == 0)
            {
                BDBG_ERR(("Invalid option tag : %s", argv[i]));
                return WRONG_USAGE;
            }
        }
        else
        {
            continue;
        }
    }

    return NO_ERROR;
}


void print_options(CLArgs *args, int len)
{
    int j;

    for (j = 0; j < len; j++)
    {
        BDBG_ERR(("Tag:%s \t Set:%d \t Value:%s", args[j].tag, args[j].set, (args[j].val == NULL) ? "" : args[j].val));
    }
}


int get_option(char *tag, CLArgs *args, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        if (strcmp(tag, args[i].tag) == 0)
        {
            return args[i].set;
        }
    }
    return 0;
}

char* get_option_value(char *tag, CLArgs *args, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        if (strcmp(tag, args[i].tag) == 0)
        {
            return args[i].val;
        }
    }
    return NULL;
}

void parse_packet_numbers(char *ptr, int *array, int *num_msgs, int max_msgs)
{
    unsigned int i, rd_index = 0;
    int count = 1, num;
    char buf[256];

    strncpy (&buf[0], ptr, 256);
    for (i = 0; i < strlen(buf); i++)
    {
        if (buf[i] == ':')
        {
            buf[i] = ' ';
            num = strtol(&buf[rd_index], NULL, 10);
            rd_index = i+1;
            array[count - 1] = num;
            count++;
        }
        if (count == max_msgs)
        {
            break;
        }
    }

    if (count < max_msgs)
    {
        num = strtol(&buf[rd_index], NULL, 10);
        array[count - 1] = num;
    }
    *num_msgs = count;
}
