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

#ifndef _PLAYLIST_H__
#define _PLAYLIST_H__

#include "resource.h"
#include "mhash.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetEngine;
class CChannel;

class CPlaylistData
{
public:
    CPlaylistData(
            const char * strIp,
            int          nIndex = 0
            ) :
        _strIp(strIp),
        _nIndex(nIndex) {}
public:
    MString _strIp;
    int     _nIndex;
};

class CPlaylist : public CMvcModel
{
public:
    CPlaylist(const char * name) :
        CMvcModel(name),
        _pId(NULL),
        _pWidgetEngine(NULL)
    {
    }

    CPlaylist(const CPlaylist & playlist) :
        CMvcModel(playlist._strName),
        _pId(playlist._pId),
        _pWidgetEngine(playlist._pWidgetEngine)
    {
        /* channel list and metadata list will NOT be copied */
    }

    virtual ~CPlaylist(void);
    virtual eRet open(CWidgetEngine * pWidgetEngine);
    virtual void close(void);
    virtual eRet copyObservers(CSubject * pSubject);

    eRet            addChannel(CChannel * pChannel);
    eRet            removeChannel(CChannel * pChannel);
    CChannel *      getChannel(int index)                          { return(_channelList.at(index)); }
    int             numChannels(void)                              { return(_channelList.total()); }
    CWidgetEngine * getWidgetEngine(void)                          { return(_pWidgetEngine); }
    void            setWidgetEngine(CWidgetEngine * pWidgetEngine) { _pWidgetEngine = pWidgetEngine; }
    void            setId(void * pId)                              { _pId = pId; }
    void *          getId(void)                                    { return(_pId); }
    void            addMetadata(
            const char * strTag,
            const char * strValue
            ) { _metadata.add(strTag, new MString(strValue)); }
    int          totalMetadata(void) { return(_metadata.total()); }
    const char * getMetadataTag(int index);
    const char * getMetadataValue(int index);
    void         dump(bool bForce = false, int index = 0);

protected:
    void *              _pId;
    CWidgetEngine *     _pWidgetEngine;
    MAutoList<CChannel> _channelList;
    MHash<MString>      _metadata;
};

#ifdef __cplusplus
}
#endif

#endif /* _PLAYLIST_H__ */