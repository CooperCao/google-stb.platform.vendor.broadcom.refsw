/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(
        int    argc,
        char * argv[],
        char * envv[]
        )
{
    int         idx      = 0;
    char *      contents = NULL;
    FILE *      fpInput  = NULL;
    struct stat statbuf;
    char *      pQueryString = NULL; /* CAD debug */

    for (idx = 0; argv[idx]; idx++)
    {
        /*fprintf( stderr, "%d %s - argv[%d] = (%s)\n", getpid(), argv[0], idx, argv[idx] );*/
    }

    if (argc < 2)
    {
        printf("{ \"Usage\":\"%s expects argv[1] to be the URL from the browser.\"}", argv[0]);
        goto error;
    }

    if (lstat(argv[1], &statbuf) == -1)
    {
        /*fprintf( stderr, "%s: Could not stat (%s)\n", __FUNCTION__, argv[1]);*/
        return(0);
    }

    contents = malloc(statbuf.st_size + 1);
    if (contents == NULL) { return(0); }

    /*
     * pQueryString = getenv( "QUERY_STRING" );
     * if ( pQueryString ) fprintf( stderr, "%s - QUERY_STRING (%s)\n", __FUNCTION__, pQueryString );
     */

    memset(contents, 0, statbuf.st_size + 1);

    if (statbuf.st_size)
    {
        fpInput = fopen(argv[1], "r");
        fread(contents, 1, statbuf.st_size, fpInput);
        fclose(fpInput);
        contents[statbuf.st_size] = '\0';
    }

    write(STDOUT_FILENO, contents, statbuf.st_size);
    fprintf(stderr, "%d %s - returned (%lu) bytes for file (%s)\n", getpid(), argv[0], (long unsigned int) statbuf.st_size, argv[1]);

#if 0
    for (idx = 0; envv[idx]; idx++)
    {
        fprintf(stderr, "%s - envv[%d] = (%s)\n", argv[0], idx, envv[idx]);
    }
    usleep(100000);
#endif /* if 0 */
    if (contents) { free(contents); }

error:

    /*fprintf( stderr, "%d %s - exiting\n", getpid(), argv[0] );*/
    return(0);
} /* main */