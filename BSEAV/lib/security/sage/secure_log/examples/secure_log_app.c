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


#include <stdio.h> /* printf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read */
#include <string.h>
#include <getopt.h>
/* open etc */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bsagelib_types.h"
#include "sage_srai.h"
#include "nexus_platform.h"
#include "nexus_sage.h"
#include "nexus_core_utils.h"
#include "secure_log_tl.h"
#include "bsagelib_types.h"
#include "nexus_security_client.h"
#if defined(RUN_AS_NXCLIENT)
#include "nxclient.h"
#endif

BDBG_MODULE(secure_log_app);

#define _STEP(STR) {printf("Press ENTER: %s\n", STR); getchar();}

#define UTIL_MODULE 0x1

#define Utility_ModuleId_eHeartbeat 0x01

static SRAI_ModuleHandle hUtilModule;
static SRAI_PlatformHandle hUtilPlatform;

static char log_name[512];
static char ta_name[512];
static char cert_name[512];

#define RING_BUFFER_SIZE_BYTES  (1024)

static int SAGE_app_join_nexus(void)
{
    int rc = 0;
#if defined(RUN_AS_NXCLIENT)
    NxClient_JoinSettings joinSettings;
    NxClient_GetDefaultJoinSettings(&joinSettings);
    joinSettings.ignoreStandbyRequest = true;
    joinSettings.timeout = 60;
    rc = NxClient_Join(&joinSettings);
    if (rc) {
       BDBG_LOG(("\tfailed to join nexus.  aborting.\n"));
       rc = 1;
    }
#else
    NEXUS_PlatformSettings platformSettings;
    BDBG_LOG(("\tBringing up all Nexus modules for platform using default settings\n\n"));
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    if(NEXUS_Platform_Init(&platformSettings) == NEXUS_SUCCESS)
    {
        BDBG_LOG(("\tNexus has initialized successfully\n"));
        rc = 0;
    }else
    {
        BDBG_ERR(("\tFailed to bring up Nexus\n"));
        rc = 1;
    }
#endif
    return rc;
}

static void SAGE_app_leave_nexus(void)
{
#if defined(RUN_AS_NXCLIENT)
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif
}

int SAGE_Util_Init(void)
{
    BERR_Code sage_rc;
    BSAGElib_State state;
    int rc = 0;

    BDBG_WRN(("\n **************START************"));

    /* Open the platform first */
    sage_rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_UTILITY,
                                 &state,
                                 &hUtilPlatform);
    if (sage_rc != BERR_SUCCESS) {
        rc = 1;
        goto end;
    }

    /* Check init state */
    if (state != BSAGElib_State_eInit) {
        /* Not yet initialized: send init command*/
        sage_rc = SRAI_Platform_Init(hUtilPlatform, NULL);
        if (sage_rc != BERR_SUCCESS) {
            rc = 2;
            goto end;
        }
    }

    sage_rc = SRAI_Module_Init(hUtilPlatform,
                               UTIL_MODULE,
                               NULL,
                               &hUtilModule);
    if (sage_rc != BERR_SUCCESS) {
        rc = 3;
        goto end;
    }

end:
    if (rc) {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);

        BDBG_WRN(("\n **************END************"));

        /* error: cleanup platform */
        Secure_Log_TlUninit();
    }
    return rc;
}

BERR_Code
SAGE_Heartbeat_Takepulse()
{
    BERR_Code rc;
    BSAGElib_InOutContainer *container = SRAI_Container_Allocate();

    BDBG_ASSERT(container);

    rc = SRAI_Module_ProcessCommand(hUtilModule,
                                    Utility_ModuleId_eHeartbeat,
                                    container);

    return rc;
}

uint32_t utilBinSize;
uint8_t *utilBinBuff;

uint32_t certBinSize;
uint8_t *certBinBuff;

BERR_Code SAGE_Utility_Platform_Install(uint8_t* logBinBuff, uint32_t logBinSize)
{
    BERR_Code sage_rc;
    int rc = 0;

    sage_rc = SRAI_Platform_Install(BSAGE_PLATFORM_ID_UTILITY, logBinBuff, logBinSize);
    if (sage_rc != BERR_SUCCESS) {
        rc = 1;
    }
    return rc;
}

static void CompleteCallback ( void *pParam, int iParam )
{
    BSTD_UNUSED(iParam);
    fprintf(stderr, "CompleteCallback:%#lx fired\n", (unsigned long)pParam);
    BKNI_SetEvent(pParam);
}

/* Common Buffer Testing */
static void
_usage(void)
{
    printf("usage:\n");
    printf("-t < Util TA > -c < Certificate BRCM USEC > -l < Log File >\n");
}

static int
SAGE_Read_File(const char *filePath, uint8_t** binBuff, uint32_t* binSize)
{
    int rc = 0;
    int fd = -1;
    uint32_t size = 0;
    uint8_t *buff;

    fd = open(filePath, O_RDONLY);
    if (fd == -1) {
        rc = -2;
        goto end;
    }

    /* poll size, allocate destination buffer and return pos to start */
    {
        off_t pos = lseek(fd, 0, SEEK_END);
        if (pos == -1) {
            rc = -3;
            goto end;
        }
        size = (uint32_t)pos;
        buff = SRAI_Memory_Allocate(size, SRAI_MemoryType_Shared);
        if (buff == NULL) {
            rc = -4;
            goto end;
        }

        pos = lseek(fd, 0, SEEK_SET);
        if (pos != 0) {
            rc = -5;
            goto end;
        }
    }

    /* read file in memory */
    {
        ssize_t readSize = read(fd, (void *)buff, (size_t)size);
        if (readSize != (ssize_t)size) {
            rc = -6;
            goto end;
        }
    }

end:
    if (fd != -1) {
        close(fd);
        fd = -1;
    }

    if (rc) {
        if (buff) {
            SRAI_Memory_Free(buff);
            buff = NULL;
        }
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    *binBuff = buff;
    *binSize = size;
    return rc;
}

static int
_parse_cmdline(int argc, char *argv[])
{
    int arg;
    int rc = 0;
    uint8_t chk = 0;

    while ((arg = getopt(argc, argv, "t:c:l:")) != -1) {
    switch (arg) {
    case 't':
       sprintf(ta_name, "%s", optarg);
       chk |= 0x1;
       break;
    case 'c':
       sprintf(cert_name, "%s", optarg);
       chk |= 0x2;
       break;
    case 'l':
       sprintf(log_name, "%s", optarg);
       chk |= 0x4;
       break;
    default:
       _usage();
       return -1;
    }
    }

    if ((chk & 0x7) != 0x7) {
       _usage();
       rc = -1;
       goto end;
    }

    rc = SAGE_Read_File(ta_name, &utilBinBuff, &utilBinSize);
    rc = SAGE_Read_File(cert_name, &certBinBuff, &certBinSize);

    if (rc) { goto end; }

end:
    return rc;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    //Dumping the Buffer into File
    uint8_t *pB64LogBuffer=NULL;
    uint32_t platId = 0;
    uint8_t *secType = NULL;
    uint32_t bufferLen = 0;
    FILE *sagelogptr = NULL;

    /* Join Nexus: Initialize platform ... */
    SAGE_app_join_nexus();

    if (_parse_cmdline(argc, argv)) {
        rc = -1;
        goto handle_err;
    }

    sagelogptr = fopen(log_name, "wb+");
    if(!sagelogptr) {
            BDBG_ERR(("\nCan not create a sage_log file"));
            rc = -1;
            goto handle_err;
    }

    //Get Buffer after logging
    Secure_Log_TlBufferContext *pBuffContext = NULL;
    uint8_t *pGlrBuffAddr = NULL;

    _STEP("Install 'Secure Log TA' platform");
    if (Secure_Log_TlInit()) {
        rc = -1;
        goto handle_err;
    }

    // Start Sage Logging
    // Attach SSF Plat Id = 0

    pBuffContext = (Secure_Log_TlBufferContext*)SRAI_Memory_Allocate(sizeof(Secure_Log_TlBufferContext), SRAI_MemoryType_Shared);
    BKNI_Memset(pBuffContext,0,sizeof(Secure_Log_TlBufferContext));

    pGlrBuffAddr = (uint8_t *)SRAI_Memory_Allocate(RING_BUFFER_SIZE_BYTES, SRAI_MemoryType_Shared);
    BKNI_Memset(pGlrBuffAddr,0,RING_BUFFER_SIZE_BYTES);

    _STEP("Start Test Case 1: platId = 0x0 : SSF, Sectype = Broadcom Common Buff");
    bufferLen = RING_BUFFER_SIZE_BYTES;
    platId = 0x0;

    // Buffer Size and Certificate
    Secure_Log_TlConfigureBuffer(certBinBuff,certBinSize,bufferLen);
    Secure_Log_TlAttach(platId);

    _STEP("Start Test Case 2 : Install and Open Util module and Log it platId = 0x103, Sectype = Broadcom Common Buff");
    SAGE_Utility_Platform_Install(utilBinBuff,utilBinSize);
    SAGE_Util_Init();
    platId = BSAGE_PLATFORM_ID_UTILITY;
    bufferLen = RING_BUFFER_SIZE_BYTES;

    Secure_Log_TlAttach(platId);
    SAGE_Heartbeat_Takepulse();
    SAGE_Heartbeat_Takepulse();
    SAGE_Heartbeat_Takepulse();
    SAGE_Heartbeat_Takepulse();
    SAGE_Heartbeat_Takepulse();
    SAGE_Heartbeat_Takepulse();

    Secure_Log_TlGetBuffer(pBuffContext,pGlrBuffAddr,RING_BUFFER_SIZE_BYTES,platId);

    // If at least one byte of data is retrieved or Get Buffer API doesn't fail
    if((*(uint8_t*)pGlrBuffAddr) != 0){
        fwrite((uint8_t*)pBuffContext ,sizeof(*pBuffContext),1,sagelogptr);
        fwrite((uint8_t*)pGlrBuffAddr,RING_BUFFER_SIZE_BYTES,1,sagelogptr);
    }

    Secure_Log_TlDetach(platId);

handle_err:

    /* Free all resource */
    if(sagelogptr) {
        fflush(sagelogptr);
        fclose(sagelogptr);
        sagelogptr=NULL;
    }

    if(pB64LogBuffer) {
    NEXUS_Memory_Free(pB64LogBuffer);
    pB64LogBuffer = NULL;
    }

    Secure_Log_TlUninit();
    SAGE_app_leave_nexus();
    return rc;
}
