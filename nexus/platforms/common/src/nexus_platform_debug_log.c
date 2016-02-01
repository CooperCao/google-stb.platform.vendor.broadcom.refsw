/***************************************************************************
*     (c)2011-2014 Broadcom Corporation
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
* API Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "nexus_platform_debug_log.h"
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

BDBG_MODULE(nexus_platform_debug_log);

#define DEFAULT_LOGGER "./logger"
#define DEFAULT_LOGGER_FILE "/tmp/nexus.log"

void NEXUS_Platform_P_DebugLog_Init(NEXUS_Platform_P_DebugLog *debugLog, const char *driver)
{
    BDBG_Fifo_CreateSettings logSettings;
    const char *debug_log_size;
    const char *logger;
    const char *fname;
    int urc;
    BERR_Code rc;
    struct stat st;
    int pipesfd[2];
    char fd[4];

    /* Allow override of logger */
    logger = NEXUS_GetEnv("nexus_logger");
    if ( NULL == logger ) {
        logger = DEFAULT_LOGGER;
    } else if ( !strcmp(logger, "disabled") ) {
        BDBG_WRN(("Logger disabled via nexus_logger=disabled"));
        goto err_logger;
    } else {
        BDBG_WRN(("Logger overridden to %s", logger));
    }

    fname = NEXUS_GetEnv("nexus_logger_file");
    if ( NULL == fname ) {
        fname = DEFAULT_LOGGER_FILE;
    } else if ( strlen(fname) >= sizeof(debugLog->fname) ) {
        BDBG_WRN(("Logger file path too long - reverting to default"));
        fname = DEFAULT_LOGGER_FILE;
    } else {
        BDBG_WRN(("Logger data file overridden to %s", fname));
    }

    /* coverity[fs_check_call: FALSE] */
    urc = lstat(logger, &st);
    if(urc<0) {goto err_logger;}

    b_strncpy(debugLog->fname, fname, sizeof(debugLog->fname));
    BDBG_Fifo_GetDefaultCreateSettings(&logSettings);
    BDBG_Log_GetElementSize(&logSettings.elementSize);
    logSettings.nelements = 1024;
    debug_log_size = NEXUS_GetEnv("debug_log_size");
    if (debug_log_size) {
        logSettings.nelements = 2*NEXUS_atoi(debug_log_size);
        if(logSettings.nelements && logSettings.nelements<8) {
            logSettings.nelements = 2048;
        }
    }
    if(logSettings.nelements==0) {
        goto err_elements;
    }
    debugLog->logSize = logSettings.nelements;
    unlink(debugLog->fname);
    debugLog->fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, 0666);
    if(debugLog->fd<0) { (void)BERR_TRACE(NEXUS_OS_ERROR);goto err_fd;}
    urc = fcntl(debugLog->fd, F_GETFL);
    if (urc != -1) {
        urc = fcntl(debugLog->fd, F_SETFL, urc | FD_CLOEXEC);
    }
    if (urc) BERR_TRACE(urc); /* keep going */
    debugLog->bufferSize = (logSettings.nelements+1) *  logSettings.elementSize;

    urc = ftruncate(debugLog->fd, debugLog->bufferSize);
    if(urc<0) { (void)BERR_TRACE(NEXUS_OS_ERROR);goto err_truncate;}

    debugLog->shared = mmap64(0, debugLog->bufferSize, PROT_READ|PROT_WRITE, MAP_SHARED, debugLog->fd, 0);
    if(debugLog->shared==MAP_FAILED) { (void)BERR_TRACE(NEXUS_OS_ERROR);goto err_mmap;}

    urc = pipe(pipesfd);
    if(urc<0) { (void)BERR_TRACE(NEXUS_OS_ERROR);goto err_pipe;}

    logSettings.buffer = debugLog->shared;
    logSettings.bufferSize = debugLog->bufferSize;
     /* coverity[ tainted_data : FALSE ] */
    rc = BDBG_Fifo_Create(&debugLog->logWriter, &logSettings);
    if(rc!=BERR_SUCCESS) {(void)BERR_TRACE(rc);goto err_fifo;}
    debugLog->logger = fork();
    if(debugLog->logger<0) {
        goto err_fork;
    } else if(debugLog->logger==0) {
        /* child */
        char *argv[5];
        close(pipesfd[0]);

        BKNI_Snprintf(fd,sizeof(fd),"%d",pipesfd[1]);
        argv[0] = (void *)logger;
        argv[1] = debugLog->fname;
        argv[2] = driver?(char *)driver:"";
        argv[3] = fd;
        argv[4] = NULL;
        /* coverity[toctou: FALSE] */
        execv(logger, argv);
        /* only reached on error */
        _exit(-1);
        /* shouldn't reach here */
        return;
    }
    close(pipesfd[1]);
    read(pipesfd[0],fd,1); /* wait for logger to reply */
    close(pipesfd[0]);
    BDBG_Log_SetFifo(debugLog->logWriter);

    return;

err_fork:
    BDBG_Fifo_Destroy(debugLog->logWriter);
    debugLog->logWriter = NULL;
err_fifo:
    close(pipesfd[0]);
    close(pipesfd[1]);
err_pipe:
    munmap(debugLog->shared, debugLog->bufferSize);
err_mmap:
err_truncate:
    unlink(debugLog->fname);
    close(debugLog->fd);
err_fd:
err_elements:
err_logger:
    return;
}

void NEXUS_Platform_P_DebugLog_Uninit(NEXUS_Platform_P_DebugLog *debugLog)
{
    int status;
    pid_t target;

    if(debugLog->logWriter==NULL) {
        return;
    }
    BKNI_Sleep(10); /* there is no any good way to synchronize with the writer */
    BDBG_Log_SetFifo(NULL);
    status = kill(debugLog->logger, SIGUSR1);
    if (status) {
        BDBG_ERR(("kill of logger failed with %d", errno));
    }
    target = waitpid(debugLog->logger, &status, 0);
    BDBG_MSG(("%d waitpid:%d, %d", (int)target, (int)debugLog->logger, (int)status));
    BDBG_Fifo_Destroy(debugLog->logWriter);
    munmap(debugLog->shared, debugLog->bufferSize);
    close(debugLog->fd);
    return;
}

