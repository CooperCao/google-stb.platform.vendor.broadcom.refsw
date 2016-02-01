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
#include "bipc_client.h"

#include "bsspk_decoder.h"
#include "bsac_session.h"
#include "bipc_test.h"

BDBG_MODULE(bipc_client_test);

#include "ipc_client.h"

static const bipc_interface_descriptor * const interfaces[] = {
   &bipc_bsac_session_descriptor,
   &bipc_bsspk_decoder_descriptor
};


int main(int argc, const char *argv[])
{
    bipc_t ipc;
    bsac_session_t  session, session2;
    bsac_session_client_configuration config;
    bsspk_decoder_t decoder, decoder2;
    int rc;
    bsac_session_client_info info;
    bsspk_decoder_resources resources;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
    BKNI_Init();
    BDBG_Init();
    BDBG_LOG(("Start"));

    BKNI_Snprintf(config.name.string, sizeof(config.name.string), "%s", argv[0]); 

    ipc = bipc_client_create(CLIENT_RECV_FD, CLIENT_SEND_FD, interfaces, sizeof(interfaces)/sizeof(*interfaces));
    BDBG_ASSERT(ipc);
    session = bsac_session_create(ipc, &config);
    BDBG_ASSERT(session);
    rc = bsac_session_get_clientinfo(session, &info);
    BDBG_LOG(("%#x %s(%u)", info.client, info.certificate.data, info.certificate.length));
    BDBG_ASSERT(rc==0);

    decoder = bsspk_decoder_open(ipc);
    BDBG_ASSERT(decoder);
    rc = bsspk_decoder_get_resources(decoder, &resources);
    BDBG_ASSERT(rc==0);

    session2 = bsac_session_create(ipc, &config);
    decoder2 = bsspk_decoder_open(ipc);

    bsac_session_destroy(session);
    bsspk_decoder_close(decoder);
    bipc_client_destroy(ipc);

    close(CLIENT_RECV_FD);
    close(CLIENT_SEND_FD);

    BDBG_Uninit();
    BKNI_Uninit();
    return 0;
}



