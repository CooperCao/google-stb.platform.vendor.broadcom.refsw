/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "band.h"
#include "convert.h"
#include "xmltags.h"
#include "encode.h"
#include "mxmlparser.h"
#include "board.h"
#include <unistd.h> /* file i/o */
#include <fcntl.h>
#include "bkni.h"

#if NEXUS_HAS_VIDEO_ENCODER

BDBG_MODULE(atlas_encode);
#define NULL_PID  0x1fff

CEncode::CEncode(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_encode, pCfg),
    _allocated(false),
    _currentVideo(NULL),
    _encoder(NULL),
    _mixer(NULL),
    _pDecodeParserBand(NULL),
    _pRecordParserBand(NULL),
    _pRecord(NULL),
    _pRecpump(NULL),
    _pBoardResources(NULL),
    _pVideoDecode(NULL),
    _pAudioDecode(NULL),
    _pStc(NULL),
    _pTranscodeStc(NULL),
    _pOutputAudioDummy(NULL),
    _pPlaybackList(NULL),
    _pModel(NULL)
{
    /* Need to init */

    _number = number;
    _pCfg   = pCfg;
    _pidMgr.dump();
    _pidMgr.initialize(pCfg);
    memset(&_encoderServerSettings, 0, sizeof(_encoderServerSettings));
    memset(&_startSettings, 0, sizeof(NEXUS_SimpleEncoderStartSettings));
    memset(&_encoderSettings, 0, sizeof(NEXUS_SimpleEncoderSettings));
    memset(_pPlaypump, 0, sizeof(_pPlaypump));
}

CEncode::~CEncode()
{
    close();
}

void CEncode::close()
{
    if (_allocated)
    {
        simple_encoder_destroy();

        if (_encoderServerSettings.videoEncoder != NULL)
        {
            NEXUS_VideoEncoder_Close(_encoderServerSettings.videoEncoder);
            _encoderServerSettings.videoEncoder = NULL;
        }

        if (_encoder != NULL)
        {
            NEXUS_SimpleEncoder_Destroy(_encoder);
            _encoder = NULL;
        }
    }

    _allocated = false;
} /* close */

void CEncode::dump()
{
    _pidMgr.dump();
    if (NULL != _currentVideo)
    {
        BDBG_MSG(("current video:"));
        _currentVideo->dump();
    }
}

void CEncode::simple_encoder_destroy(void)
{
    unsigned i;

    if (_encoderServerSettings.audioMuxOutput)
    {
        NEXUS_AudioMuxOutput_Destroy(_encoderServerSettings.audioMuxOutput);
        _encoderServerSettings.audioMuxOutput = NULL;
    }

    for (i = 0; i < NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS; i++)
    {
        if (_pPlaypump[i])
        {
            _pPlaypump[i]->close();
            _pBoardResources->checkinResource((CResource *)(_pPlaypump[i]));
            _pPlaypump[i] = NULL;
        }
    }

    if (_encoderServerSettings.streamMux)
    {
        NEXUS_StreamMux_Destroy(_encoderServerSettings.streamMux);
        _encoderServerSettings.streamMux = NULL;
    }

    if (_pTranscodeStc != NULL)
    {
        _pTranscodeStc->close();
        _pTranscodeStc                             = NULL;
        _encoderServerSettings.stcChannelTranscode = NULL;
    }
} /* simple_encoder_destroy */

eRet CEncode::simple_encoder_create()
{
    NEXUS_DisplaySettings         displaySettings;
    NEXUS_StreamMuxCreateSettings streamMuxCreateSettings;
    unsigned                      i;
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    NEXUS_VideoEncoderCapabilities cap;

    if (_encoder == NULL)
    {
        _encoder = NEXUS_SimpleEncoder_Create(_pModel->getSimpleEncoderServer(), _number);
        CHECK_PTR_ERROR_GOTO("cannot get encoder ", _encoder, ret, eRet_NotAvailable, error);
    }

    NEXUS_SimpleEncoder_GetServerSettings(_pModel->getSimpleEncoderServer(), _encoder, &_encoderServerSettings);

    NEXUS_Display_GetDefaultSettings(&displaySettings);

    displaySettings.displayType     = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.frameRateMaster = NULL; /* disable frame rate tracking for now */
    NEXUS_GetVideoEncoderCapabilities(&cap);
    _encoderServerSettings.transcodeDisplayIndex = cap.videoEncoder[_number].displayIndex;
    BDBG_MSG(("Opening Display %d", _encoderServerSettings.transcodeDisplayIndex));
    /* window is opened internally */

#if 0 /* DONOT */
    _encoderServerSettings.displayEncode.display = mainDisplay;
#endif
    /* Will be in another Class for 14.3 */
    _encoderServerSettings.audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
    CHECK_PTR_ERROR_GOTO("Cannot open Audio Mux ", _encoderServerSettings.audioMuxOutput, ret, eRet_NotAvailable, error);

    {
        NEXUS_AudioMixerSettings mixerSettings;

        NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
        mixerSettings.mixUsingDsp = true;
        _mixer                    = NEXUS_AudioMixer_Open(&mixerSettings);
        CHECK_PTR_ERROR("_mixer issue", _mixer, ret, eRet_ExternalError);
    }

    _encoderServerSettings.mixer = _mixer;

    _pOutputAudioDummy->disconnect();

    NEXUS_AudioOutput_AddInput(_pOutputAudioDummy->getConnectorA(), NEXUS_AudioMixer_GetConnector(_mixer));
    if (_encoderServerSettings.videoEncoder == NULL)
    {
        _encoderServerSettings.videoEncoder = NEXUS_VideoEncoder_Open(_number, NULL);
        CHECK_PTR_ERROR_GOTO("Cannot open VideoEndcoder ", _encoderServerSettings.videoEncoder, ret, eRet_NotAvailable, error);
    }

    for (i = 0; i < NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS; i++)
    {
        NEXUS_PlaypumpOpenSettings playpumpConfig;

        BDBG_MSG(("trying to Check out playpump %d", i));
        /* Check out playpumps necessary */
        _pPlaypump[i] = (CPlaypump *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_playpump);
        CHECK_PTR_ERROR_GOTO("Unable to open playpump", _pPlaypump[i], ret, eRet_ExternalError, error);

        playpumpConfig                     = _pPlaypump[i]->getPlaypumpOpenSettings();
        playpumpConfig.fifoSize            = 16384; /* reduce FIFO size allocated for playpump */
        playpumpConfig.numDescriptors      = 64;    /* set number of descriptors */
        playpumpConfig.streamMuxCompatible = true;
        _pPlaypump[i]->setSettings(playpumpConfig);

        ret = _pPlaypump[i]->open();
        CHECK_ERROR_GOTO("cannot open playpump", ret, error);
        _encoderServerSettings.playpump[i] = _pPlaypump[i]->getPlaypump();
        BDBG_MSG(("Success Checked out playpump %d", i));
    }

    /* Needs its own class */
    NEXUS_StreamMux_GetDefaultCreateSettings(&streamMuxCreateSettings);
    _encoderServerSettings.streamMux = NEXUS_StreamMux_Create(&streamMuxCreateSettings);

    _encoderServerSettings.stcChannelTranscode = _pTranscodeStc->getStcChannel();

    nerror = NEXUS_SimpleEncoder_SetServerSettings(_pModel->getSimpleEncoderServer(), _encoder, &_encoderServerSettings);
    CHECK_NEXUS_ERROR_GOTO("SimpleEncoder SetServer Settings failed", ret, nerror, error);

    BDBG_MSG(("Success in creating Encoder Server!!!"));

error:

    return(ret);
} /* simple_encoder_create */

eRet CEncode::initialize()
{
    eRet ret = eRet_Ok;

    ret = CResource::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    NEXUS_SimpleEncoder_GetDefaultStartSettings(&_startSettings);

    BDBG_MSG(("added Encode #%d", _number));
    goto done;

error:
    if (NULL != _encoder)
    {
        BDBG_ERR(("Cannot Open Encoder"));
        _encoder = NULL;
    }

done:
    return(ret);
} /* initialize */

eRet CEncode::createVideo()
{
    eRet ret = eRet_Ok;

    /* Create Video PID */

    CPid * pPid = NULL;

    pPid = new CPid(_startSettings.output.video.pid, NEXUS_VideoCodec_eH264);
    CHECK_PTR_ERROR_GOTO("unable to create pid", pPid, ret, eRet_OutOfMemory, error);
    _currentVideo->getPidMgr()->addPid(pPid);

    pPid = new CPid(_startSettings.output.audio.pid, NEXUS_AudioCodec_eAac);
    CHECK_PTR_ERROR_GOTO("unable to create pid", pPid, ret, eRet_OutOfMemory, error);
    _currentVideo->getPidMgr()->addPid(pPid);

    pPid = new CPid(_startSettings.output.transport.pcrPid, ePidType_Pcr);
    CHECK_PTR_ERROR_GOTO("unable to create pid", pPid, ret, eRet_OutOfMemory, error);
    _currentVideo->getPidMgr()->addPid(pPid);

    BDBG_MSG(("create audio/video pids"));
error:
    return(ret);
} /* createVideo */

/* Private function to close handles!! */
void CEncode::checkinResources()
{
    /* Lots of cleanup still todo !!*/
    if (NULL != _pRecord)
    {
        _pRecord->close();
        _pBoardResources->checkinResource(_pRecord);
        _pRecord = NULL;
        /* No need to check in Recpump it is tied to record */
        _pRecpump = NULL;
    }

    if (NULL != _pAudioDecode)
    {
        _pAudioDecode->stop();
    }

    if (NULL != _pVideoDecode)
    {
        _pVideoDecode->stop();
    }
    if (_mixer)
    {
        NEXUS_AudioMixer_Close(_mixer);
        _mixer                       = NULL;
        _encoderServerSettings.mixer = NULL;
    }

    if (NULL != _pOutputAudioDummy)
    {
        _pOutputAudioDummy->disconnect();
        _pBoardResources->checkinResource(_pOutputAudioDummy);
        _pOutputAudioDummy = NULL;
    }

    if (NULL != _pRecordParserBand)
    {
        _pRecordParserBand->close();
        _pBoardResources->checkinResource(_pRecordParserBand);
        _pRecordParserBand = NULL;
    }

    if (NULL != _pStc)
    {
        _pStc->close();
        _pBoardResources->checkinResource(_pStc);
        _pStc = NULL;
    }

    simple_encoder_destroy();

    if (_pAudioDecode)
    {
        _pAudioDecode->close();
        _pBoardResources->checkinResource(_pAudioDecode);
        _pAudioDecode = NULL;
    }
    if (NULL != _pVideoDecode)
    {
        _pVideoDecode->close();
        _pBoardResources->checkinResource(_pVideoDecode);
        _pVideoDecode = NULL;
    }
} /* checkinResources */

eRet CEncode::open()
{
    eRet ret = eRet_Ok;

    BDBG_MSG(("Open Encode %d!", _number));
    _pVideoDecode = (CSimpleVideoDecode *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_simpleDecodeVideo);
    CHECK_PTR_ERROR_GOTO("Unable to open video decode", _pVideoDecode, ret, eRet_ExternalError, error);

    _pStc = (CStc *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_stcChannel);
    CHECK_PTR_ERROR_GOTO("Unable to open STC channel", _pStc, ret, eRet_ExternalError, error);

    _pVideoDecode->setResources(getCheckedOutId(), _pBoardResources);

    ret = _pStc->open();
    CHECK_ERROR_GOTO("stc failed to open", ret, error);

    ret = _pVideoDecode->open(NULL, _pStc);
    CHECK_ERROR_GOTO("video decode failed to open", ret, error);

    /* Transcode STC configuring once we know what STC value there is */
    {
        /* need to call STC checkout! */
        NEXUS_StcChannelSettings stcSettings;
        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        /* should be the same timebase for end-to-end locking */
        stcSettings.timebase = (NEXUS_Timebase) _pVideoDecode->getNumber();
        stcSettings.mode     = NEXUS_StcChannelMode_eAuto;
        stcSettings.pcrBits  = NEXUS_StcChannel_PcrBits_eFull42; /* ViCE2 requires 42-bit STC broadcast */

        _pTranscodeStc = (CStc *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_stcChannel);
        CHECK_PTR_ERROR_GOTO("Unable to checkout Regular transcode STC channel", _pTranscodeStc, ret, eRet_ExternalError, error);
        ret = _pTranscodeStc->open(stcSettings);
        CHECK_ERROR_GOTO("Unable to open Regular transcode STC channel", ret, error);
        /* Need to reconfigure the Transcode STC to match the */
    }

    _pRecordParserBand = (CParserBand *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_parserBand);
    CHECK_PTR_ERROR_GOTO("Unable to checkout Record ParserBand", _pRecordParserBand, ret, eRet_ExternalError, error);

    _pAudioDecode = (CSimpleAudioDecode *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_simpleDecodeAudio);
    CHECK_PTR_ERROR_GOTO("Unable to open audio decode", _pAudioDecode, ret, eRet_ExternalError, error);

    _pOutputAudioDummy = (COutputAudioDummy *) _pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_outputAudioDummy);
    CHECK_PTR_ERROR_GOTO("Unable to open audio decode", _pOutputAudioDummy, ret, eRet_ExternalError, error);

    _pAudioDecode->setResources(getCheckedOutId(), _pBoardResources);

    /* give audio outputs to simple audio decoder */
    _pAudioDecode->setOutputDummy(_pOutputAudioDummy);

    ret = _pAudioDecode->open(NULL, _pStc);
    CHECK_ERROR_GOTO("Unable to open audio decode", ret, error);

    /* add record */
    _pRecord = (CRecord *)_pBoardResources->checkoutResource(getCheckedOutId(), eBoardResource_record);
    CHECK_PTR_ERROR_GOTO("unable to checkout Record", _pRecord, ret, eRet_NotAvailable, done);

    /* Board Resources needs to be passed down in order to checkout recpumps */
    _pRecord->setResources(NULL, _pBoardResources);
    ret = _pRecord->open(); /* must be done after passing in Board Resources*/
    CHECK_ERROR_GOTO("Unable to open record", ret, error);

    _pRecpump = _pRecord->getRecpump();

    ret = simple_encoder_create();
    CHECK_ERROR_GOTO("Unable to open encoder", ret, error);

    /* Get the Default settings for this encoder channel */
    NEXUS_SimpleEncoder_GetSettings(_encoder, &_encoderSettings);

    _allocated = true;

    BDBG_MSG(("Encode Open #%d with all resources!", _number));

    goto done;
error:
    checkinResources();
    _allocated = false;

done:
    return(ret);
} /* initialize */

eRet CEncode::openPids(
        CPidMgr * pPidMgr,
        bool      record
        )
{
    CPid * pVideoPid = pPidMgr->getPid(0, ePidType_Video);
    CPid * pAudioPid = pPidMgr->getPid(0, ePidType_Audio);
    CPid * pPcrPid   = pPidMgr->getPid(0, ePidType_Pcr);
    eRet   ret       = eRet_Ok;

    /*
     * Move this to Channel Class
     * open pids -
     * Need to know if it is IP. open pids need playback
     * Maybe each channel needs to open their own pids
     */
    if (pVideoPid)
    {
        pVideoPid->setPlayback(true);
        if (record)
        {
            ret = pVideoPid->open(_pRecord);
        }
        else
        {
            ret = pVideoPid->open(_pDecodeParserBand);
        }

        CHECK_ERROR_GOTO("open video pid channel failed", ret, error);
    }

    if (pAudioPid)
    {
        pAudioPid->setPlayback(true);
        if (record)
        {
            ret = pAudioPid->open(_pRecord);
        }
        else
        {
            ret = pAudioPid->open(_pDecodeParserBand);
        }

        CHECK_ERROR_GOTO("open audio pid channel failed", ret, error);
    }

    if (pPcrPid)
    {
        pPcrPid->setPlayback(true);
    }

    return(ret);

error:
    /* Close Pids channels */
    if (pVideoPid)
    {
        if (record)
        {
            pVideoPid->close(_pRecord);
        }
        else
        {
            pVideoPid->close();
        }
    }

    if (pAudioPid)
    {
        if (record)
        {
            pAudioPid->close(_pRecord);
        }
        else
        {
            pAudioPid->close();
        }
    }

    return(ret);
} /* openPids */

eRet CEncode::start()
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    if ((_currentVideo == NULL) || _currentVideo->getVideosPath().isEmpty())
    {
        BDBG_ERR(("Please setVideo(CVideo *Video) before calling %s ", __FUNCTION__));
        return(eRet_ExternalError);
    }

    _pRecord->setBand(_pRecordParserBand);
    /* Open the pids from PidMgr for Decode  */
    ret = openPids(&_pidMgr, false);
    CHECK_ERROR_GOTO("Cannot Open Pids for Transcode!", ret, error);

    /* Configure STC*/
    _pStc->setStcType(eStcType_ParserBand);
    ret = _pStc->configure(_pidMgr.getPid(0, ePidType_Video));
    CHECK_ERROR_GOTO("unable to configure stc", ret, error);

    /* Finish StartSettings */
    _startSettings.input.video = _pVideoDecode->getSimpleDecoder();
    _startSettings.input.audio = _pAudioDecode->getSimpleDecoder();
    _startSettings.recpump     = _pRecpump->getRecpump();
    /* No Index for now */
    _startSettings.output.video.index = false;
    _startSettings.input.display      = false;

    ret = createVideo();
    CHECK_ERROR_GOTO("Cannot create CVideo transcoded Pids!", ret, error);

    setSettings(_encoderSettings);
    nerror = NEXUS_SimpleEncoder_Start(_encoder, &_startSettings);
    CHECK_NEXUS_ERROR_GOTO("Cannot Start Simple Encoder", ret, nerror, error);

    BDBG_MSG(("Start Simple Encoder ... Now start video/audio + Record"));

    /*
     * Sleep
     * BKNI_Sleep(100);
     */

    ret = _pVideoDecode->start(_pidMgr.getPid(0, ePidType_Video), _pStc);
    CHECK_ERROR_GOTO("Cannot Start Simple Video Decoder", ret, error);

    ret = _pAudioDecode->start(_pidMgr.getPid(0, ePidType_Audio), _pStc);
    CHECK_ERROR_GOTO("Cannot Start Simple Audio Decoder", ret, error);

    /*
     * Sleep
     * BKNI_Sleep(100);
     */

    _pRecord->setVideo(_currentVideo);
    _pRecord->start();

    BDBG_MSG(("Record Started"));

    CHECK_PTR_ERROR_GOTO("must setPlaybackList first", _pPlaybackList, ret, eRet_NotAvailable, error);

    _pPlaybackList->addVideo(_currentVideo, 0);
    _pPlaybackList->createInfo(_currentVideo);
    _pPlaybackList->sync();

    ret = notifyObservers(eNotify_EncodeStarted, this);
    CHECK_ERROR_GOTO("error notifying observers", ret, error);

    BDBG_MSG(("Encode Started"));
    return(ret);

error:

    if (_pRecord)
    {
        _pRecord->stop();
    }

    if (_encoder)
    {
        NEXUS_SimpleEncoder_Stop(_encoder);
    }

    return(ret);
} /* start */

void CEncode::dupPidMgr(CPidMgr * pPidMgr)
{
    BDBG_ASSERT(NULL != pPidMgr);
    /* pPidMgr->dump(); */
    _pidMgr = *pPidMgr;
    /* _pidMgr->dump(); */
}

eRet CEncode::setSettings(NEXUS_SimpleEncoderSettings encoderSettings)
{
    NEXUS_Error nerror    = NEXUS_SUCCESS;
    eRet        ret       = eRet_Ok;
    int         frameRate = 0;

    /* NEXUS_SimpleEncoder_GetSettings(_encoder,&encoderSettings); */
    _encoderSettings = encoderSettings;

    _encoderSettings.videoEncoder.bitrateMax = 3*1000*1000; /* 3Mbps(unsigned) GET_INT(_pCfg, TRANSCODE_VIDEOBITRATE); */
    _encoderSettings.video.width             = 1280;        /* GET_INT(_pCfg, TRANSCODE_VIDEOWIDTH); */
    _encoderSettings.video.height            = 720;         /* GET_INT(_pCfg, TRANSCODE_VIDEOWIDTH); */

    frameRate = 30000; /* GET_INT(_pCfg, TRANSCODE_FRAMERATE); */

    /* with 1000/1001 rate tracking by default */
    if ((frameRate == 29970) || (frameRate == 30000))
    {
        _encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e30;
    }
    else
    if ((frameRate == 59940) || (frameRate == 60000))
    {
        _encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e60;
        _encoderSettings.video.refreshRate      = 60000;
    }
    else
    if ((frameRate == 23976) || (frameRate == 24000))
    {
        _encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e24;
    }
    else
    if ((frameRate == 14985) || (frameRate == 15000))
    {
        _encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e14_985;
    }
    else
    if (frameRate == 50000)
    {
        _encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e50;
        _encoderSettings.video.refreshRate      = 50000;
    }
    else
    if (frameRate == 25000)
    {
        _encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e25;
        _encoderSettings.video.refreshRate      = 50000;
    }

    _encoderSettings.video.interlaced = false; /* GET_BOOL(_pCfg, TRANSCODE_INTERLACED); */
    nerror                            = NEXUS_SimpleEncoder_SetSettings(_encoder, &_encoderSettings);
    CHECK_NEXUS_ERROR_GOTO("Simple Encoder Set Settings", ret, nerror, error);

    return(ret);

error:
    BDBG_ERR((" Error in Set Settings"));
    return(ret);
} /* setSettings */

/* Is Active */
bool CEncode::isActive(void)
{
    if ((_currentVideo != NULL))
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

eRet CEncode::stop()
{
    eRet ret = eRet_Ok;

    BDBG_MSG(("Encode stop"));
    if (_encoder)
    {
        NEXUS_SimpleEncoder_Stop(_encoder);
    }

    _pidMgr.closePidChannels();
    _pidMgr.clearPids();

    checkinResources();
    _currentVideo = NULL;

    ret = notifyObservers(eNotify_EncodeStopped, this);
    CHECK_ERROR_GOTO("error notifying observers", ret, error);

error:
    return(ret);
} /* stop */

CPid * CEncode::getPid(
        uint16_t index,
        ePidType type
        )
{
    BDBG_ASSERT(NULL != _currentVideo);
    return(_currentVideo->getPidMgr()->getPid(index, type));
}

#endif /* NEXUS_HAS_VIDEO_ENCODER */