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
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

#if __cplusplus >= 201103L
#error "code is not yet ready for C++14"
#endif

#include "pr_decryptor.h"
#ifdef MSDRM_PRDY30
#include "pr30_decryptor.h"
#endif
#ifdef ENABLE_WIDEVINE
#include "wv_decryptor.h"
#endif
#ifdef ENABLE_WIDEVINE_3x
#include "wv3x_decryptor.h"
#endif

BDBG_MODULE(decryptor_factory);

using namespace dif_streamer;

IDecryptor* DecryptorFactory::CreateDecryptor(DrmType type)
{
    IDecryptor* decryptor = NULL;
    switch(type) {
      case drm_type_ePlayready:
        decryptor = new PlayreadyDecryptor();
        break;
      case drm_type_ePlayready30:
#ifdef MSDRM_PRDY30
        decryptor = new Playready30Decryptor();
#else
        LOGE(("%s: Playready 3.0 not supported with this build", BSTD_FUNCTION));
#endif
        break;
#ifdef ENABLE_WIDEVINE
      case drm_type_eWidevine:
        decryptor = new WidevineDecryptor();
        break;
#endif
#ifdef ENABLE_WIDEVINE_3x
      case drm_type_eWidevine3x:
        decryptor = new Widevine3xDecryptor();
        break;
#endif
      case drm_type_eClear:
        LOGW(("%s: clear content", BSTD_FUNCTION));
        break;
      default:
        LOGE(("%s: unknown DRM type", BSTD_FUNCTION));
    }
    return decryptor;
}

void DecryptorFactory::DestroyDecryptor(IDecryptor* decryptor)
{
    LOGD(("%s: decryptor=%p", BSTD_FUNCTION, (void*)decryptor));
    if (decryptor == NULL) {
        LOGE(("%s: decryptor is NULL", BSTD_FUNCTION));
        return;
    }
    delete decryptor;
}
