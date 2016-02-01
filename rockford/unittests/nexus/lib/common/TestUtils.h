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
 * Module Description: Generic test utils header for test apps
 *
 *************************************************************/

/* Error Types */
#define NO_ERROR                    0
#define ERROR                       -1
#define WRONG_USAGE                 -2
#define PARSE_ERROR                 -3
#define INVALID_ARGUMENT            -4
#define TIMEOUT_ERROR               -5
#define FILE_IO_ERROR               -6

#define OUT_OF_MEMORY_ERROR         -10
#define NEXUS_INIT_FAILURE          -11

#define RETURN_TEST_RESULT(A)               \
    do {                                    \
        if (expected_retval != NO_ERROR) {  \
            printf("Expecting non-zero retval : %d\n", expected_retval);    \
        }                                   \
        if (A == expected_retval) {         \
            return NO_ERROR;                \
        } else {                            \
            printf("Expected retval = %d : Actual retval = %d\n", expected_retval, A);  \
            return A;                       \
        }                                   \
    } while (0)

/* Command line argument info */
typedef struct CLArgs
{
    char *tag;              /* Command tag */
    int set;                /* If the value has been set after parsing command line arguments */
    int val_required;       /* If a value is required following the command tag */
    char *val;              /* Value provided with the command tag */
    char *description;      /* Brief description of the command line argument */
} CLArgs;

/* Test case info structure */
typedef struct TestCaseInfo
{
	/* Test case description */
	char *name;
	/* Test case function pointer */
	int (*fn)(void);
} TestCaseInfo;

#define OPTIONS_PARSE(A, B)     options_parse(A, B, args, sizeof(args)/sizeof(CLArgs))
#define PRINT_OPTIONS()         print_options(args, sizeof(args)/sizeof(CLArgs))
#define GET_OPTION(A)           get_option(A, args, sizeof(args)/sizeof(CLArgs))
#define GET_OPTION_VALUE(A)     get_option_value(A, args, sizeof(args)/sizeof(CLArgs))

/* Parses command line arguments and stores the info in CLArgs structure
 * use GET_OPTION and GET_OPTION_VALUE macros to get the value for the
 * command line arguments */
int options_parse(int argc, char **argv, CLArgs *args, int len);

/* Prints the command line arguments structure populated by parsing argv[] */
void print_options(CLArgs *args, int len);

/* Returns 1 if the command line argument tag was specified, else returns 0 */
int get_option(char *tag, CLArgs *args, int len);

/* Returns the pointer to the argument following the command line argument tag */
char* get_option_value(char *tag, CLArgs *args, int len);

/* Parses colon separated numbers in a string and returns the array of integers along with the count */
void parse_packet_numbers(char *ptr, int *array, int *num_msgs, int max_msgs);
