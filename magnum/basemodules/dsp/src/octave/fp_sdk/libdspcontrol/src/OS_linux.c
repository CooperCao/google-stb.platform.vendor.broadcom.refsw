/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

#include "libdspcontrol/CHIP.h"

#if !(FEATURE_IS(SW_HOST, LINUX)          || \
      FEATURE_IS(SW_HOST, RAAGA_ROCKFORD) || \
      IS_TARGET(RaagaFP4015_si_magnum_permissive) || \
      IS_HOST(BM)                         || \
      IS_HOST(DSP_LESS))
#  error "This module is only for Linux and Raaga Rockford (and you are not building for BM or workstation either)"
#endif


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "libdspcontrol/DSPLOG.h"
#include "libdspcontrol/OS.h"



void OS_pathSearch(const char *file, char *result, int maxResultLen)
{
    /* Make an obvious check on lengths */
    int fileLen = strlen(file);
    char *path;
    char *end;
    int folderLen;
    int fd;
    if(fileLen + 1 > maxResultLen)
    {
        DSPLOG_ERROR("OS: filename length %d is greater than the buffer length %d, this will never work!",
                fileLen + 1, maxResultLen);
    }

    /* First check the file string itself */
    strncpy(result, file, maxResultLen);
    result[maxResultLen - 1] = '\0';    /* safety first */
    {
        int fd = OS_open(result, O_RDONLY);
        if(fd >= 0)
        {
            OS_close(fd);
            return;
        }
    }

    /*  Get the PATH and iterate over each element */
    path = getenv("PATH");
    DSPLOG_JUNK("OS: pathSearch got PATH = %s", path);
    while(*path)
    {
        /*  Find the next delimiter or end of string */
        end = strchr(path, ':');
        if(!end)
            end = strchr(path, '\0');

        /*  Make a temp filename */
        folderLen = end - path;
        /*  will it fit? */
        if(folderLen + fileLen + 2 > maxResultLen)  /*  1 for the '/' and 1 'for the termination '/0' */
        {
            DSPLOG_ERROR("OS: pathSearch used destination buffer size is too small, current = %d, at least %d required",
                         maxResultLen, folderLen + fileLen + 1);
            goto next_element;      /*  I'll go to hell for this, I know.... */
        }
        /*  it will fit */
        strncpy(result, path, folderLen);
        result[folderLen] = '/';
        strcpy(result + folderLen + 1, file);
        DSPLOG_JUNK("OS: checking existence of %s", result);

        /*  Does it exist? */
        fd = OS_open(result, O_RDONLY);
        if(fd >= 0)
        {
            OS_close(fd);
            return;
        }

        next_element:
        if(*end == '\0')
            break;
        path = end + 1;     /*  next element in PATH */
    }

    /*  Nothing found on PATH */
    *result = '\0';
}


int OS_spawn(const char *path, char * const argv[], pid_t *child_pid)
{
    int filedes[2];
    int rv = pipe(filedes);
    int maxfd = 512;    /*  sysconf(_SC_OPEN_MAX); */
    int i;
    pid_t child;

    if(rv == -1)
        FATAL_ERROR("OS: couldn't create pipe");

    child = fork();
    if(child < 0)
    {
        FATAL_ERROR("OS: fork doom");
    }
    else if(child == 0)
    {
        /*  We are the child */
        close(filedes[1]);
        dup2(filedes[0], STDIN_FILENO);

        /*  Close all other fds */
        /*  TODO: find a better method.... */
        for(i = 3; i < maxfd; i++)
            close(i);

        execv(path, argv);
        FATAL_ERROR("OS: launch of %s failed: %s", path, strerror(errno));
    }
    else
    {
        /*  We are the parent */
        close(filedes[0]);

        /*  Store child PID if requested */
        if(child_pid != NULL)
            *child_pid = child;
    }

    return filedes[1];
}


long OS_getCmdLine(pid_t pid, char *result, int maxResultLen)
{
    /*
     * In Linux, cmdline is to be used as a regular file which splits arguments by NULL chars.#
     * So, if you have three arguments named 1.foo 2.bar 3.example, then cmdline will have "foo\0bar\0example\0".
     * From http://www.unix.com/unix-advanced-expert-users/86740-retrieving-command-line-arguments-particular-pid.html
     */
    char filename[128];
    int fd;
    long cmdline_length;
    ssize_t r;
    int dataLength;
    int i;

    snprintf(filename, sizeof(filename), "/proc/%d/cmdline", pid);

    /*  read command line from /proc/PID/cmdline */
    fd = open(filename, O_RDONLY);
    cmdline_length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    r = read(fd, result, maxResultLen);
    close(fd);

    /*  error? */
    if(r <= 0)
    {
        snprintf(result, maxResultLen, "Error reading command line for process");
        return -1;
    }

    /*  NULL-terminate the string in any case */
    dataLength = r;
    if(dataLength == maxResultLen)
        dataLength = maxResultLen - 1;
    result[dataLength] = 0;

    /*  join the string by replacing inner NULLs with spaces */
    for(i = 0; i < dataLength - 1; i++)
        if(result[i] == 0)
            result[i] = ' ';

    return cmdline_length;
}


pid_t OS_getParentPid(pid_t pid)
{
    /*
     * Retrieve the parent pid.
     * See man 5 proc
     */
    char filename[128];
    char buffer[1024];
    int fd;
    ssize_t r;
    pid_t ppid;

    snprintf(filename, sizeof(filename), "/proc/%d/stat", pid);

    /*  read process statistics from /proc/PID/stat */
    fd = open(filename, O_RDONLY);
    r = read(fd, buffer, sizeof(buffer));
    close(fd);

    /*  error? */
    if(r <= 0)
        return -1;

    /*  NULL-terminate the string in any case */
    if(r == sizeof(buffer))
        buffer[sizeof(buffer) - 1] = 0;
    else
        buffer[r] = 0;

    /*  read the parent pid (4th field). Use filename as dummy mem area. */
    if(4 > sscanf(buffer, "%d %s %c %d", (int *) filename, filename, filename, &ppid))
        return -1;
    else
        return ppid;
}


void OS_sleep(unsigned int time_ms)
{
    struct timespec ts;
    ts.tv_sec = time_ms / 1000;
    ts.tv_nsec = (time_ms % 1000) * 1000000;

    nanosleep(&ts, 0);
}


size_t OS_timeAsString(char *dst, size_t n, char *format)
{
    struct tm tt;

#if _POSIX_TIMERS > 0
    struct timespec tv;
    if(clock_gettime(CLOCK_REALTIME, &tv) == 0)
#else
    struct timeval tv;
    struct timezone tz;
    if(gettimeofday(&tv, &tz) == 0)
#endif
    {
        localtime_r(&tv.tv_sec, &tt);
        return strftime(dst, n, format, &tt);
    }
    else
    {
        if(n > 0)
            *dst = '\0';
        return 0;
    }
}
