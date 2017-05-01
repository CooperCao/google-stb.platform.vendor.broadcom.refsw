/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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

 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strtok, ...
#include <stdbool.h>
#include <fcntl.h> // open, ...
#include <sys/types.h> // size_t, ...
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

const char carriage_ret[] = "\r";
/**
 *  Function: This function will send the specified buffer to the specified socket.
 **/
void bmemperf_send_data(
     int socket_fd,
     const char *const Caller,
     int iswrite,
     const char *tagname,
     const char *data,
     int len
     )
{

    /*printf("%s: %s Sending data; [%s]\n", Caller, DateYyyyMmDdHhMmSs(), data);*/
    if (write(socket_fd, data, sizeof(char)*(len+1)) < 0) {
        perror( "bmemperf_send_data; sending username");
        return;
    }

    /* Input a carriage return character to complete data send */
    if (write(socket_fd, carriage_ret, sizeof(char)*1) < 0) {
        perror( "bmemperf_send_data; sending username");
        return;
    }
}

/**
 *  Function: This function will read the string response from the specified socket.
 **/
int bmemperf_get_response (
    const char * const Caller,
    int socket_fd,
    const char * const ExpectedResponse,
    char * ActualResponse,
    int ActualResponseLen )
{
    fd_set               master_fdset; /* master_fdset file descriptor list */
    fd_set               read_fdset;   /* temp file descriptor list for select() */
    int                  fdmax;        /* maximum file descriptor number */
    char                 buf[1024];    /* buffer for client data */
    int                  nbytes;
    int                  i;

    /*printf("%s: from %s ... socket_fd %d ... look for (%s)\n", __FUNCTION__, Caller, socket_fd, ExpectedResponse );*/
    /* clear the master_fdset and temp sets */
    FD_ZERO(&master_fdset);
    FD_ZERO(&read_fdset);

    FD_SET(socket_fd, &master_fdset);
    fdmax = socket_fd;    /* so far, it's this one */

    /* loop */
    for (;;)
    {
        /* copy it */
        read_fdset = master_fdset;

        /*printf("fdmax is %d...\n", fdmax);*/
        if (select(fdmax + 1, &read_fdset, NULL, NULL, NULL) == -1) {
            perror("Server-select() error lol!");
            exit(1);
        }
        /*printf("Server-select() is OK...\n");*/

        /*
         * run through the existing connections looking for data to
         * be read
         */
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fdset)) {    /* we got one... */
                {
                    memset( buf, 0, sizeof(buf) );

                    /* handle data from a client */
                    if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
                        /* got error or connection closed by client */
                        if (nbytes == 0)
                            /* connection closed */
                            printf("%s: socket %d hung up\n", Caller, i);

                        else
                            perror("recv() error lol!");

                        /* close it... */
                        close(i);
                        /* remove from master_fdset set */
                        FD_CLR(i, &master_fdset);
                    } else {
                        /* we got some data from a client */

                        if ( nbytes > 0 )
                        {
                            long int response_len = 0;
                            strncat( ActualResponse, buf, ActualResponseLen - strlen(ActualResponse) - 1 );
                            response_len = strlen(ActualResponse);
                            /*printf("Response now is len %ld (%s) \n", response_len, ActualResponse);
                            fflush(stdout);fflush(stderr);*/
                            if ( response_len > 5)
                            {
                                if ( response_len > 8 &&
                                     (ActualResponse[response_len-2] == ':' || ActualResponse[response_len-2] == '#' ) &&
                                     (ActualResponse[response_len-1] == ' ' ) )
                                {
                                    /*printf( "ActualResponse ... len %d ... (%s) ... returning\n==========\n", strlen(ActualResponse), ActualResponse );*/
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                /*printf("fd %d is not ready\n", i );*/
            }
        }
    }
    return 0;
}
