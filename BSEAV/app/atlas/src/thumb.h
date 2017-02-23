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

#ifndef THUMB_H__
#define THUMB_H__

#include "atlas.h"
#include "atlas_cfg.h"
#include "atlas_os.h"

#include "bioatom.h"
#include "bfile_stdio.h"
#include "bfile_buffer.h"
#include "bthumbnail_extractor.h"
#include "nexus_surface.h"

class CPlaypump;
class CVideo;
class CStillDecode;
class CModel;
class CConfig;
class CBoardResources;
class CGraphics;

#ifdef __cplusplus
extern "C" {
#endif

class CThumb
{
public:
    CThumb(CConfiguration * pCfg);
    ~CThumb(void);

    eRet create(void);
    void destroy(void);
    eRet open(CVideo * pVideo, CStillDecode * pStillDecode, CGraphics * pGraphics);
    void close(void);
    eRet extract(uint32_t timeStamp, NEXUS_SurfaceHandle stillSurface, uint16_t width, uint16_t height);
    eRet getStillSurface(CStillDecode * pStillDecode, CGraphics * pGraphics, NEXUS_SurfaceHandle stillSurface, uint16_t width, uint16_t height);

    void dump(void);

    void * getId(void) { return(_pId); }
    void   setResources(
            void *            pId,
            CBoardResources * pResources
            ) { _pId = pId; _pBoardResources = pResources; }
    void          setModel(CModel * pModel)    { _pModel = pModel; }
    B_EventHandle getEvent(void)               { return(_eventStillReady); }
    void          setConfig(CConfig * pConfig) { _pConfig = pConfig; }
    CConfig *     getConfig(void)              { return(_pConfig); }

protected:
    CPlaypump *            _pPlaypump;
    CBoardResources *      _pBoardResources;
    void *                 _pId;
    CConfig *              _pConfig;
    CConfiguration *       _pCfg;
    CModel *               _pModel;
    bthumbnail_extractor_t _extractor;
    CVideo *               _pVideo;
    FILE *                 _fpData;
    FILE *                 _fpIndex;
    bfile_io_read_t        _bfpData;
    bfile_io_read_t        _bfpIndex;
    B_EventHandle          _eventStillReady;
    CStillDecode *         _pStillDecode;
    CGraphics *            _pGraphics;
};

#ifdef __cplusplus
}
#endif

#endif /* THUMB_H__ */