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
#include "playlist.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"
#include "channel_bip.h"

BDBG_MODULE(atlas_playlist);

CPlaylist::~CPlaylist()
{
}

eRet CPlaylist::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    _pWidgetEngine = pWidgetEngine;

    return(ret);
}

void CPlaylist::close()
{
    _pWidgetEngine = NULL;
}

eRet CPlaylist::copyObservers(CSubject * pSubject)
{
    eRet       ret      = eRet_Ok;
    CChannel * pChannel = NULL;

    CMvcModel::copyObservers(pSubject);

    /* propogate registered observers to channel list */
    for (pChannel = _channelList.first(); pChannel; pChannel = _channelList.next())
    {
        ret = pChannel->copyObservers(pSubject);
        CHECK_ERROR_GOTO("unable to copy playlist observers to its channels", ret, error);
    }

error:
    return(ret);
} /* copyObservers */

/* adds new channels linked to given owner (pId).  only the owner who added
 * the channel is authorized to remove it */
eRet CPlaylist::addChannel(CChannel * pChannelNew)
{
    eRet       ret      = eRet_Ok;
    CChannel * pChannel = NULL;

    BDBG_ASSERT(NULL != pChannelNew);

    for (pChannel = _channelList.first(); pChannel; pChannel = _channelList.next())
    {
        if (pChannel == pChannelNew)
        {
            /* found matching channel */
            break;
        }
    }

    if (NULL != pChannel)
    {
        /* TODO: update channel data */
    }
    else
    {
        /* add new channel */
        _channelList.add(pChannelNew);
    }

    return(ret);
} /* addChannel */

/* remove channel matching given channel. only the given owner that matches the intended
 * channel may remove it */
eRet CPlaylist::removeChannel(CChannel * pChannelRemove)
{
    eRet       ret      = eRet_NotAvailable;
    CChannel * pChannel = NULL;

    BDBG_ASSERT(NULL != pChannelRemove);

    for (pChannel = _channelList.first(); pChannel; pChannel = _channelList.next())
    {
        if (pChannel == pChannelRemove)
        {
            /* found matching channel */
            break;
        }
    }

    if (NULL != pChannel)
    {
        _channelList.remove(pChannel);
        ret = eRet_Ok;
    }

    return(ret);
} /* removeChannel */

const char * CPlaylist::getMetadataTag(int index)
{
    const char * pStr = _metadata.getName(index);

    return(pStr);
}

const char * CPlaylist::getMetadataValue(int index)
{
    MString * pStr = NULL;

    pStr = _metadata.getData(index);
    if (NULL != pStr)
    {
        return(pStr->s());
    }
    return(NULL);
}

void CPlaylist::dump(
        bool bForce,
        int  index
        )
{
    BDBG_Level    level;
    CChannelBip * pChannelBip = NULL;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_playlist", &level);
        BDBG_SetModuleLevel("atlas_playlist", BDBG_eMsg);
    }

    BDBG_WRN(("Playlist:%s", getName().s()));

    if (0 == index)
    {
        int i = 1;
        for (pChannelBip = (CChannelBip *)_channelList.first(); pChannelBip; pChannelBip = (CChannelBip *)_channelList.next())
        {
            BDBG_WRN(("%d %s %d", i, pChannelBip->getUrlPath().s(), pChannelBip->getProgram()));
            i++;
        }
    }
    else
    {
        pChannelBip = (CChannelBip *)_channelList[index - 1];
        if (NULL != pChannelBip)
        {
            BDBG_WRN(("%d %s %d", index, pChannelBip->getUrlPath().s(), pChannelBip->getProgram()));
        }
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_playlist", level);
    }

    /* notify observer of channel shown (Lua uses as response to request) */
    {
        /* if requested index is > 0 then return corresponding 1 index based playlist
         * with notification */
        if (0 < index)
        {
            pChannelBip = (CChannelBip *)_channelList[index - 1];
        }

        notifyObservers(eNotify_PlaylistShown, pChannelBip);
    }
} /* dump */