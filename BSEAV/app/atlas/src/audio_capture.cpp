/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "nexus_audio_capture.h"
#include "audio_capture.h"
#include "atlas_os.h"

BDBG_MODULE(atlas_audio_capture);

CAudioCapture::CAudioCapture(const char * name) :
    CMvcModel(name),
    _pWidgetEngine(NULL),
    _state(eAudioCaptureState_Off)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(eRet_Ok == ret);
}

CAudioCapture::~CAudioCapture(void)
{
    /* _clientList.clear(); */
}

eRet CAudioCapture::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(pWidgetEngine);
    BDBG_ERR(("%s:Only supported in Atlas NxClient mode. Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
}

eRet CAudioCapture::close()
{
    eRet ret = eRet_Ok;

    BDBG_ERR(("%s:Only supported in Atlas NxClient mode. Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
}

eRet CAudioCapture::start(void)
{
    eRet ret = eRet_Ok;

    BDBG_ERR(("%s:Only supported in Atlas NxClient mode. Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
}

eRet CAudioCapture::stop(void)
{
    eRet ret = eRet_Ok;

    BDBG_ERR(("%s:Only supported in Atlas NxClient mode. Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
}

eRet CAudioCapture::getBuffer(
        void **  pBuffer,
        size_t * pBufferSize
        )
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pBufferSize);
    BDBG_ERR(("%s:Only supported in Atlas NxClient mode. Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
}

eRet CAudioCapture::readComplete(unsigned bufferSize)
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(bufferSize);
    BDBG_ERR(("%s:Only supported in Atlas NxClient mode. Implementation left as stub. Implemented in NX client version  ", __FUNCTION__));
    return(ret);
}

void CAudioCapture::addClient(CAudioCaptureClient * client)
{
    _clientList.add(client);
}

void CAudioCapture::removeClient(CAudioCaptureClient * client)
{
    _clientList.remove(client);
}