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

#ifndef BOARD_H__
#define BOARD_H__

#include "mstring.h"
#include "mlist.h"
#include "atlas.h"
#include "resource.h"
#include "display.h"
#include "playback.h"
#include "record.h"
#include "audio_playback.h"

#include "encode.h"

#include "graphics.h"
#include "band.h"
#include "remote.h"

#if NEXUS_HAS_FRONTEND
#include "tuner.h"
#endif

#ifdef MPOD_SUPPORT
#include "cablecard.h"
#endif

#if  defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
#include "dma.h"
#endif

#include "timebase.h"
#include "nexus_video_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WPA_SUPPLICANT_SUPPORT
class CWifi;
#endif
#ifdef NETAPP_SUPPORT
class CNetwork;
class CBluetooth;
#endif
class CServerMgr;

class CChipFamily
{
public:
    CChipFamily(void);
    ~CChipFamily(void);
    bool operator ==(const CChipFamily &other);
protected:
    MString  _name;
    MString  _revision;
    char     _major;
    unsigned _minor;
};

class CChipInfo
{
public:
    CChipInfo(void);
    ~CChipInfo(void);

    eRet         setNumber(uint32_t number);
    uint32_t     getNumber(void) { return(_number); }
    eRet         setRevision(const char * revision);
    const char * getRevision(void) { return(_revision.s()); }
    char         getMajor()        { return(_major); }
    unsigned     getMinor()        { return(_minor); }

    eRet addFamily(CChipFamily family);
    eRet removeFamily(CChipFamily family);

    void dump();

    bool operator ==(const CChipInfo &other);

protected:
    uint32_t            _number;
    MString             _revision;
    char                _major;
    unsigned            _minor;
    MList <CChipFamily> _familyList;
};

class CBoardFeatures
{
public:
    CBoardFeatures(void);
    ~CBoardFeatures(void);

    void clear();
    void dump(bool bForce = false);

public:
    bool _videoHd;       /* system could decode HD video */
    bool _displayHd;     /* system could display HD video */
    bool _dnrDcr;        /* system might have DCR support (digital contour removal) */
    bool _dnrBnr;        /* system might have BNR support (block noise removal) */
    bool _dnrMnr;        /* system might have MNR support (mosquito noise removal)*/
    bool _anr;           /* system might have ANR support (analog noise reduction)*/
    bool _deinterlacer;  /* system might have deinterlacing support */
    int  _boxDetect;     /* system has letterbox detection support */
    bool _pvrEncryption; /* system might have PVR encryption */
    bool _sharpness;     /* System might have sharpness control */
    bool _cab;           /* System might have color adjustments [Green stretch, Blue Stretch, Auto Flesh Tome] */
    bool _lab;           /* System might have luma adjustments [Dynamic Contrast] */
    bool _mosaic;        /* System might support mosaic mode */
    bool _autoVolume;    /* System has Auto Volume Level post processing */
    bool _dolbyVolume;   /* System has Dolby volume post processing */
    bool _srsVolume;     /* System has SRS volume IQ post processing */

    bool _videoFormatIsSupported[NEXUS_VideoFormat_eMax]; /* Array of supported video formats (index w/bvideo_format values) */
};

#define ANY_INDEX   0xFFFF
#define ANY_NUMBER  0xFFFFFFFF

class CBoardResources
{
public:
    CBoardResources();
    ~CBoardResources();

    virtual eRet add(eBoardResource resource, const unsigned numResources, const char * name, CConfiguration * pCfg,
            const unsigned startIndex = 0, const unsigned id = 0);

#if NEXUS_HAS_FRONTEND
    virtual eRet addFrontend(const unsigned numTuner, CConfiguration * pCfg, NEXUS_FrontendCapabilities * pCapabilities = NULL);
#endif

    void clear(void);

    CResource * checkoutResource(void * id, eBoardResource resource, unsigned index = ANY_INDEX, uint32_t number = ANY_NUMBER);
    eRet        checkinResource(CResource * pResource);
    CResource * reserveResource(void * id, eBoardResource resource, unsigned index = ANY_INDEX);
    CResource * findCheckedoutResource(void * id, eBoardResource resource, unsigned index = ANY_INDEX);
    bool        findResource(void * id, eBoardResource resource, unsigned index = ANY_INDEX);

    eRet registerObserver(void * id, eBoardResource resource, CObserver * pObserver, unsigned index = ANY_INDEX, eNotification notification = eNotify_All);
    eRet unregisterObserver(void * id, eBoardResource resource, CObserver * pObserver, unsigned index = ANY_INDEX, eNotification notification = eNotify_All);

    void dump(bool bForce = false);
    void dumpList(MList <CResource> * pList);

protected:
    MAutoList <CDisplay>           _displayList;           /* list of displays */
    MAutoList <CGraphics>          _graphicsList;          /* list of graphics */
    MAutoList <CSurfaceClient>     _surfaceClientList;     /* list of surface clients */
    MAutoList <CVideoDecode>       _decodeVideoList;       /* list of video decodes */
    MAutoList <CSimpleVideoDecode> _simpleDecodeVideoList; /* list of simple video decodes */
    MAutoList <CAudioDecode>       _decodeAudioList;       /* list of audio decodes */
    MAutoList <CSimpleAudioDecode> _simpleDecodeAudioList; /* list of simple audio decodes */
    MAutoList <CSimplePcmPlayback> _simplePcmPlaybackList; /* list of simple audio playbacks */
    MAutoList <CStc>               _stcChannelList;        /* list of stc channels */
    MAutoList <CResource>          _pcmPlaybackList;       /* list of PCM playbacks */
    MAutoList <CResource>          _pcmCaptureList;        /* list of PCM capture channels */
    MAutoList <CResource>          _decodeStillList;       /* list of video still picture decodes */
    MAutoList <CResource>          _decodeEsList;          /* list of ES only decoders (used or audio ES decode)*/
    MAutoList <CResource>          _decodeMosaicList;      /* list of mosaic mode decoders */
    MAutoList <CResource>          _streamerList;          /* list of streamer inputs */
    MAutoList <CResource>          _lineinList;            /* list of line-in */
    MAutoList <CRecpump>           _recpumpList;           /* list of records */
    MAutoList <CRecord>            _recordList;            /* list of records */
    MAutoList <CResource>          _recordPesList;         /* list of PES records */
    MAutoList <CResource>          _recordTsdmaList;       /* list of TSDMA records */
#if  defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    MAutoList <CDma> _dmaList; /* list of DMA channels */
#endif
    MAutoList <CTimebase> _timebaseList; /* list of timebases */
    MAutoList <CPlaypump> _playpumpList; /* list of playpumps */
    MAutoList <CPlayback> _playbackList; /* list of playbacks */
#if DVR_LIB_SUPPORT
    MAutoList <CTsb> _tsbList; /* list of tsbs */
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    MAutoList <CEncode> _encodeList; /* list of MPEG encoders */
#endif
    MAutoList <CInputBand>         _inputBandList;         /* list of transport input bands */
    MAutoList <CParserBand>        _parserBandList;        /* list of transport parser bands */
    MAutoList <COutputSpdif>       _outputSpdifList;       /* list of SPDIF outputs */
    MAutoList <COutputAudioDac>    _outputAudioDacList;    /* list of audio dac outputs */
    MAutoList <COutputAudioDacI2s> _outputAudioDacI2sList; /* list of audio dac outputs */
    MAutoList <COutputAudioDummy>  _outputAudioDummyList;  /* list of audio dummy outputs */
    MAutoList <COutputComponent>   _outputComponentList;   /* list of Component outputs */
    MAutoList <COutputSVideo>      _outputSVideoList;      /* list of SVideo outputs */
    MAutoList <COutputComposite>   _outputCompositeList;   /* list of composite outputs */
    MAutoList <COutputRFM>         _outputRFMList;         /* list of RFM outputs */
    MAutoList <COutputHdmi>        _outputHdmiList;        /* list of Hdmi outputs */
    MAutoList <CIrRemote>          _remoteListIr;          /* list of input ir remotes */
#if RF4CE_SUPPORT
    MAutoList <CRf4ceRemote> _remoteListRf4ce; /* list of input rf4ce remotes */
#endif
#if NETAPP_SUPPORT
    MAutoList <CBluetoothRemote> _remoteListBluetooth; /* list of input bluetooth remotes */
#endif
#if NEXUS_HAS_UHF_INPUT
    MAutoList <CUhfRemote> _remoteListUhf; /* list of input uhf remotes */
#endif
    MAutoList <CPower> _powerList; /* list of power */
#ifdef WPA_SUPPLICANT_SUPPORT
    MAutoList <CWifi> _wifiList; /* list of wifi opjects */
#endif
#ifdef NETAPP_SUPPORT
    MAutoList <CNetwork>   _networkList;   /* list of network */
    MAutoList <CBluetooth> _bluetoothList; /* list of bluetooth devices */
#endif
#if NEXUS_HAS_FRONTEND
    MAutoList <CTuner> _tunerList; /* list of frontends */
#endif

    MAutoList <CResource> * _mapResourceList[eBoardResource_max]; /* resource enum to resource list mapping */
};

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H__ */