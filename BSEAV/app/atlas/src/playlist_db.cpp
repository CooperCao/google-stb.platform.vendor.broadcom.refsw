/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "playlist_db.h"
#include "atlas.h"
#include "atlas_cfg.h"
#include "convert.h"

BDBG_MODULE(atlas_playlist_db);

CPlaylistDb::~CPlaylistDb()
{
}

eRet CPlaylistDb::open(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    _pWidgetEngine = pWidgetEngine;

    return(ret);
}

void CPlaylistDb::close()
{
    _pWidgetEngine = NULL;
}

eRet CPlaylistDb::copyObservers(CSubject * pSubject)
{
    eRet        ret       = eRet_Ok;
    CPlaylist * pPlaylist = NULL;

    CMvcModel::copyObservers(pSubject);

    /* propogate registered observers to playlists */
    for (pPlaylist = _playlistList.first(); pPlaylist; pPlaylist = _playlistList.next())
    {
        ret = pPlaylist->copyObservers(pSubject);
        CHECK_ERROR_GOTO("unable to copy playlist DB observers to its playlists", ret, error);
    }

error:
    return(ret);
} /* copyObservers */

/* adds new playlist or updates data if matching playlist already exists in list.
 * only the owner may remove the added playlist */
eRet CPlaylistDb::addPlaylist(
        void *      pId,
        CPlaylist * pPlaylistNew
        )
{
    eRet        ret       = eRet_Ok;
    CPlaylist * pPlaylist = NULL;

    BDBG_ASSERT(NULL != pId);
    BDBG_ASSERT(NULL != pPlaylistNew);

    for (pPlaylist = _playlistList.first(); pPlaylist; pPlaylist = _playlistList.next())
    {
        if (pPlaylist == pPlaylistNew)
        {
            if (pPlaylist->getId() == pId)
            {
                /* found matching playlist and owner id */
                break;
            }
        }
    }

    if (NULL != pPlaylist)
    {
        /* TODO: update playlist data - may need to delete given pPlaylistNew */
        BDBG_WRN(("TTTTTTTTTTTTTT TODO: update playlist data"));
    }
    else
    {
        /* add new playlist */
        BDBG_MSG(("addPlaylist(%s)", pPlaylistNew->getName().s()));
        /* copy id and registered observers into new playlist which will
         * propogate to its channel list */
        pPlaylistNew->setId(pId);
        pPlaylistNew->copyObservers(this);

        _playlistList.add(pPlaylistNew);

        notifyObservers(eNotify_PlaylistAdded, pPlaylistNew);
    }

    return(ret);
} /* addPlaylist */

/* remove given playlist from database.
 * only the owner who added the playlist is allowed to remove it */
eRet CPlaylistDb::removePlaylist(
        void *      pId,
        CPlaylist * pPlaylistRemove
        )
{
    eRet        ret       = eRet_NotAvailable;
    CPlaylist * pPlaylist = NULL;

    BDBG_ASSERT(NULL != pId);
    BDBG_ASSERT(NULL != pPlaylistRemove);

    for (pPlaylist = _playlistList.first(); pPlaylist; pPlaylist = _playlistList.next())
    {
        if (pPlaylist == pPlaylistRemove)
        {
            if (pPlaylist->getId() == pId)
            {
                /* found matching playlist and owner id */
                break;
            }
        }
    }

    if (NULL != pPlaylist)
    {
        BDBG_MSG(("removePlaylist(%s)", pPlaylistRemove->getName().s()));
        _playlistList.remove(pPlaylistRemove);
        notifyObservers(eNotify_PlaylistRemoved, pPlaylistRemove);
        ret = eRet_Ok;
    }

    return(ret);
} /* removePlaylist */

void CPlaylistDb::dump(
        bool bForce,
        int  index
        )
{
    BDBG_Level  level;
    CPlaylist * pPlaylist = NULL;
    int         i         = 0;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_playlist_db", &level);
        BDBG_SetModuleLevel("atlas_playlist_db", BDBG_eMsg);
    }

    BDBG_MSG(("Discovered Atlas Playlists"));
    BDBG_MSG(("=========================="));
    for (pPlaylist = _playlistList.first(); pPlaylist; pPlaylist = _playlistList.next())
    {
        CChannel * pChannel = NULL;

        i++;

        if ((0 < index) && (i != index))
        {
            /* index is specified so we will only print that particular playlist */
            continue;
        }
        if (0 == pPlaylist->numChannels())
        {
            continue;
        }

        pChannel = pPlaylist->getChannel(0);
        if (NULL == pChannel)
        {
            BDBG_MSG(("discovered playlist (%s) does not have an IP address", pPlaylist->getName().s()));
            continue;
        }

        BDBG_MSG(("%s %s", pChannel->getHost().s(), pPlaylist->getName().s()));
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_playlist_db", level);
    }

    /* notify observer of playlist shown (Lua uses as response to request) */
    {
        CPlaylist * pPlaylist = NULL;

        /* if requested index is > 0 then return corresponding 1 index based playlist
         * with notification */
        if (0 < index)
        {
            pPlaylist = _playlistList[index - 1];
        }

        notifyObservers(eNotify_DiscoveredPlaylistsShown, pPlaylist);
    }
} /* dump */