/******************************************************************************
 *    (c)2011 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/
#include "bstd.h"
#include "bkni_multi.h"
#include "bipc_server.h"

#include "bsspk_decoder.h"
#include "bsac_session.h"
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "bipc_test.h"


BDBG_MODULE(bipc_server_test);

#include "ipc_server.h" 

static const bipc_server_descriptor * const interfaces[] = {
   &bipc_bsac_session_server_descriptor,
   &bipc_bsspk_decoder_server_descriptor
};

int main(int argc, const char *argv[])
{
    int recv_fd[2];
    int send_fd[2];
    int rc;
    pid_t child;
    bipc_t ipc;
    bipc_server_client_t client;
    bipc_server_create_settings settings;
    bipc_server_client_create_settings client_settings;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
    BKNI_Init();
    BDBG_Init();

    bipc_server_get_default_create_settings(&settings);
    settings.interfaces = interfaces;
    settings.interface_count = sizeof(interfaces)/sizeof(*interfaces);
    ipc = bipc_server_create(&settings);
    BDBG_ASSERT(ipc);

    rc = pipe(recv_fd);
    BDBG_ASSERT(rc==0);

    rc = pipe(send_fd);
    BDBG_ASSERT(rc==0);

    BDBG_LOG(("Fork"));
    child = fork();
    BDBG_LOG(("Fork -> %u", getpid() ));
    BDBG_ASSERT(child!=-1);

    if(child == 0) {
        /* child */
        rc = dup2(recv_fd[1], CLIENT_SEND_FD);
        BDBG_ASSERT(rc==CLIENT_SEND_FD);
        close(recv_fd[1]);
        rc = dup2(send_fd[0], CLIENT_RECV_FD);
        BDBG_ASSERT(rc==CLIENT_RECV_FD);
        close(send_fd[0]);
        rc = execlp(argv[1], argv[1], NULL);
        BDBG_ASSERT(rc==0);
        _exit(0);
    }
    close(recv_fd[1]);
    close(send_fd[0]);
    bipc_server_get_default_client_create_settings(&client_settings);
    client_settings.recv_fd = recv_fd[0];
    client_settings.send_fd = send_fd[1];
    client_settings.ipc_context = "client";
    client = bipc_server_client_create(ipc, &client_settings);
    for(;;) {
        unsigned processed;
        BDBG_LOG(("Process"));
        rc = bipc_server_client_process(ipc, client);
        if(rc!=0) {
            break;
        }
    }
    BDBG_LOG(("Done"));
    bipc_server_client_destroy(ipc, client);
    close(client_settings.recv_fd);
    close(client_settings.send_fd);
    waitpid(child, NULL, 0);
    bipc_server_destroy(ipc);
    BDBG_Uninit();
    BKNI_Uninit();
    return 0;
}


struct bsac_session {
    int unused;
};

bsac_session_t  bsac_session_create(bipc_t ipc, const bsac_session_client_configuration *config)
{
    bsac_session_t session;
    BSTD_UNUSED(ipc);
    BSTD_UNUSED(config);
    BDBG_LOG(("bsac_session_create:%s '%s'", (char *)ipc, config->name.string));
    session = BKNI_Malloc(sizeof(*session));
    BDBG_LOG(("bsac_session_create:>%#lx", (unsigned long)session ));
    return  session;
}

int   bsac_session_get_clientinfo(bsac_session_t session, bsac_session_client_info *info)
{
    BDBG_LOG(("bsac_session_get_clientinfo:%#lx %#lx", (unsigned long)session, (unsigned long)info ));
    BKNI_Memset(info, 0, sizeof(*info));
    info->client = (unsigned long)session;
    info->certificate.length = BKNI_Snprintf(info->certificate.data, sizeof(info->certificate.data), "client %#x", session);
    return 0;
}

void  bsac_session_destroy(bsac_session_t session)
{
    BDBG_LOG(("bsac_session_destroy:%#lx", (unsigned long)session ));
    BKNI_Free(session);
    return;
}

struct bsspk_decoder {
    int unused;
};

bsspk_decoder_t  bsspk_decoder_open(bipc_t ipc)
{
    bsspk_decoder_t decoder;
    BDBG_LOG(("bsspk_decoder_open:%s", (char *)ipc));
    BSTD_UNUSED(ipc);
    decoder = BKNI_Malloc(sizeof(*decoder));
    BDBG_LOG(("bsspk_decoder_open:>%#lx", (unsigned long)decoder));
    return  decoder;
}

int bsspk_decoder_get_resources(bsspk_decoder_t decoder, bsspk_decoder_resources *resources)
{
    BDBG_LOG(("bsspk_decoder_get_resources:%#lx", (unsigned long)decoder));
    BKNI_Memset(resources, 0, sizeof(*resources));
    return 0;
}

int bsspk_decoder_get_default_audio_settings(bsspk_decoder_t decoder, bsspk_decoder_audio_settings *settings)
{
    BDBG_LOG(("bsspk_decoder_get_default_audio_settings:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    BKNI_Memset(settings, 0, sizeof(*settings));
    return 0;
}

int bsspk_decoder_set_audio_settings(bsspk_decoder_t decoder, unsigned id, const bsspk_decoder_audio_settings *settings)
{
    BDBG_LOG(("bsspk_decoder_set_audio_settings:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    BSTD_UNUSED(id);
    BSTD_UNUSED(settings);
    return 0;
}

int bsspk_decoder_get_default_video_settings(bsspk_decoder_t decoder, bsspk_decoder_video_settings *settings)
{
    BDBG_LOG(("bsspk_decoder_get_default_video_settings:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    BKNI_Memset(settings, 0, sizeof(*settings));
    return 0;
}

int bsspk_decoder_set_video_settings(bsspk_decoder_t decoder, unsigned id, const bsspk_decoder_video_settings *settings)
{
    BDBG_LOG(("bsspk_decoder_set_video_settings:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    BSTD_UNUSED(id);
    BSTD_UNUSED(settings);
    return 0;
}

void bsspk_decoder_close(bsspk_decoder_t decoder)
{
    BDBG_LOG(("bsspk_decoder_close:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    return;
}

int bsspk_decoder_clock_get(bsspk_decoder_t decoder, uint32_t *clock)
{
    BDBG_LOG(("bsspk_decoder_clock_get:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    BSTD_UNUSED(clock);
    return 0;
}

int bsspk_decoder_clock_set(bsspk_decoder_t decoder, uint32_t clock)
{
    BDBG_LOG(("bsspk_decoder_clock_set:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    BSTD_UNUSED(clock);
    return 0;
}

int bsspk_decoder_clock_stop(bsspk_decoder_t decoder)
{
    BDBG_LOG(("bsspk_decoder_clock_stop:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    return 0;
}

int bsspk_decoder_clock_start(bsspk_decoder_t decoder)
{
    BDBG_LOG(("bsspk_decoder_clock_start:%#lx", (unsigned long)decoder));
    BSTD_UNUSED(decoder);
    return 0;
}
    

