/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "HdmiSource.h"
#ifdef NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#include "nexus_hdmi_output.h"
#include "nexus_platform.h"
#endif //NEXUS_HAS_HDMI_INPUT

#include <string>
#include <cstring>

namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(HdmiSource);

#ifdef NEXUS_HAS_HDMI_INPUT
struct hdmi_edid {
    const uint8_t *data;
    uint16_t size;
    bool allocated;
};

static const uint8_t sampleEdid[] = {
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x6D, 0x74, 0x22, 0x05, 0x01, 0x11, 0x20,
    0x00, 0x14, 0x01, 0x03, 0x80, 0x00, 0x00, 0x78, 0x0A, 0xDA, 0xFF, 0xA3, 0x58, 0x4A, 0xA2, 0x29,
    0x17, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
    0x45, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20,
    0x58, 0x2C, 0x25, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x9E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x42,
    0x43, 0x4D, 0x37, 0x34, 0x32, 0x32, 0x2F, 0x37, 0x34, 0x32, 0x35, 0x0A, 0x00, 0x00, 0x00, 0xFD,
    0x00, 0x17, 0x3D, 0x0F, 0x44, 0x0F, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x89,

    0x02, 0x03, 0x3C, 0x71, 0x7F, 0x03, 0x0C, 0x00, 0x40, 0x00, 0xB8, 0x2D, 0x2F, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xE3, 0x05, 0x1F, 0x01, 0x49, 0x90, 0x05, 0x20, 0x04, 0x03, 0x02, 0x07,
    0x06, 0x01, 0x29, 0x09, 0x07, 0x01, 0x11, 0x07, 0x00, 0x15, 0x07, 0x00, 0x01, 0x1D, 0x00, 0x72,
    0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A,
    0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x18,
    0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0x0B, 0x88, 0x21, 0x00,
    0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D
};
#endif //NEXUS_HAS_HDMI_INPUT

typedef struct tagBMEHDMIEngineContext {
#ifdef NEXUS_HAS_HDMI_INPUT
    struct hdmi_edid edid;
    NEXUS_HdmiInputHandle   hdmiInput;
    NEXUS_HdmiOutputHandle  hdmiOutput;
    NEXUS_HdmiInputSettings hdmiInputSettings;
#endif //NEXUS_HAS_HDMI_INPUT
    unsigned index;
} BMEHDMIEngineContext;


#ifdef NEXUS_HAS_HDMI_INPUT
static void get_hdmi_output_edid(NEXUS_HdmiOutputHandle hdmiOutput, struct hdmi_edid *edid)
{
    NEXUS_Error rc;
    uint8_t *attachedRxEdid;
    size_t attachedRxEdidSize;
    NEXUS_HdmiOutputBasicEdidData hdmiOutputBasicEdidData;
    NEXUS_HdmiOutputEdidBlock edidBlock;
    NEXUS_HdmiOutputStatus status;
    uint16_t i;
    memset(edid, 0, sizeof(*edid));
    /* default to sample edid if not connect or there's any failure */
    edid->data = sampleEdid;
    edid->size = sizeof(sampleEdid);

    if (hdmiOutput) {
        NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
    } else {
        status.connected = false;
    }

    if (!status.connected) {
        return;
    }

    rc = NEXUS_HdmiOutput_GetBasicEdidData(hdmiOutput, &hdmiOutputBasicEdidData);

    if (rc != NEXUS_SUCCESS) {
        BME_DEBUG_ERROR(("Unable to get downstream EDID; Use default EDID for repeater's EDID"));
        return;
    }

    /* allocate space to hold the EDID blocks */
    attachedRxEdidSize = (hdmiOutputBasicEdidData.extensions + 1) * sizeof(edidBlock.data);
    attachedRxEdid = (uint8_t*)malloc(attachedRxEdidSize);

    for (i = 0; i <= hdmiOutputBasicEdidData.extensions; i++) {
        uint16_t j;
        rc = NEXUS_HdmiOutput_GetEdidBlock(hdmiOutput, i, &edidBlock);

        if (rc != NEXUS_SUCCESS) {
            BME_DEBUG_ERROR(("Error retrieving EDID Block %d from attached receiver;", i));
            free(attachedRxEdid);
            return;
        }

        /* copy EDID data */
        for (j = 0; j < sizeof(edidBlock.data); j++)  {
            BDBG_ASSERT(i * sizeof(edidBlock.data) + j < attachedRxEdidSize);
            attachedRxEdid[i * sizeof(edidBlock.data) + j] = edidBlock.data[j];
        }
    }

    edid->data = attachedRxEdid;
    edid->allocated = true;
    edid->size = attachedRxEdidSize;
    return;
}

static void free_hdmi_output_edid(struct hdmi_edid *edid)
{
    if (edid->allocated && edid->data) {
        free((void*)edid->data);
        edid->data = NULL;
    }
}
#endif //NEXUS_HAS_HDMI_INPUT

HdmiSource::HdmiSource() :
    _state(IMedia::IdleState),
    _context(NULL)
{
    init();
}

HdmiSource::~HdmiSource()
{
    uninit();
}

void HdmiSource::init()
{
    BME_DEBUG_ENTER();

    if (NULL == _context) {
        _context = new BMEHDMIEngineContext();
        std::memset(_context, 0, sizeof(BMEHDMIEngineContext));
    }

    _context->index = 0;
    BME_DEBUG_EXIT();
}

void HdmiSource::uninit()
{
    BME_DEBUG_ENTER();

    if (_context == NULL)
        return;
#ifdef NEXUS_HAS_HDMI_INPUT
    if (_context->hdmiInput)
        NEXUS_HdmiInput_Close(_context->hdmiInput);
    _context->hdmiInput = NULL;

    if (_context->hdmiOutput) {
        NEXUS_HdmiOutput_Close(_context->hdmiOutput);
    }
    _context->hdmiOutput = NULL;
#endif
    delete _context;
    _context = NULL;

    _state = IMedia::IdleState;
    BME_DEBUG_EXIT();
}

void HdmiSource::start()
{
    BME_DEBUG_ENTER();
#ifdef NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInput_GetDefaultSettings(&_context->hdmiInputSettings);
    _context->hdmiInputSettings.timebase = NEXUS_Timebase_e0;
    // set hpdDisconnected to true if a HDMI switch is in front of the Broadcom HDMI Rx.
    // -- The NEXUS_HdmiInput_ConfigureAfterHotPlug should be called to inform the hw of
    // -- the current state,  the Broadcom SV reference boards have no switch so
    // -- the value should always be false
    _context->hdmiInputSettings.frontend.hpdDisconnected = false;
    // use NEXUS_HdmiInput_OpenWithEdid ()
    //    if EDID PROM (U1304 and U1305) is NOT installed;
    //    reference boards usually have the PROMs installed.
    //    this example assumes Port1 EDID has been removed
    /* all HDMI Tx/Rx combo chips have EDID RAM */
    _context->hdmiInputSettings.useInternalEdid = true;
    _context->hdmiInput = NEXUS_HdmiInput_OpenWithEdid(_context->index,
            &_context->hdmiInputSettings, (uint8_t*)_context->edid.data, _context->edid.size);

    if (!_context->hdmiInput) {
        BME_DEBUG_ERROR(("Can't get hdmi input !!!"));
        return;
    }

    free_hdmi_output_edid(&_context->edid);

    if (getConnector()->videoDecoder) {
        BME_CHECK(NEXUS_SimpleVideoDecoder_StartHdmiInput(getConnector()->videoDecoder, _context->hdmiInput, NULL));
        BME_DEBUG_TRACE(("HDMI Input video source coonected !"));
    } else {
        BME_DEBUG_ERROR(("HdmiInput %d no video available", _context->index));
        return;
    }

    if (getConnector()->audioDecoder) {
        NEXUS_SimpleAudioDecoder_StartHdmiInput(getConnector()->audioDecoder, _context->hdmiInput, NULL);
        BME_DEBUG_TRACE(("HDMI Input audio source coonected !"));
    } else {
        BME_DEBUG_ERROR(("HdmiInput %d no audio available", _context->index));
        return;
    }

    _state = IMedia::StartedState;
#else  //NEXUS_HAS_HDMI_INPUT
    BME_DEBUG_ERROR(("No HDMI Input on this board !!!!!!!!!"));
#endif //NEXUS_HAS_HDMI_INPUT
    BME_DEBUG_EXIT();
}

void HdmiSource::stop(bool holdLastFrame)
{
    TRLS_UNUSED(holdLastFrame);
#ifdef NEXUS_HAS_HDMI_INPUT
    if (_context->hdmiInput)
        NEXUS_HdmiInput_Close(_context->hdmiInput);
    _context->hdmiInput = NULL;
#endif ////NEXUS_HAS_HDMI_INPUT
    _state = IMedia::StoppedState;
}

IMedia::ErrorType HdmiSource::prepare()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_TRACE(("HdmiSource::prepare"));
#ifdef NEXUS_HAS_HDMI_INPUT
    _context->hdmiOutput = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
    get_hdmi_output_edid(_context->hdmiOutput, &_context->edid);

    if (_context->hdmiOutput) {
        NEXUS_HdmiOutput_Close(_context->hdmiOutput);
    }
    _context->hdmiOutput = NULL;

#endif //NEXUS_HAS_HDMI_INPUT
    _state = IMedia::StoppedState;
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}
void HdmiSource::setDataSource(MediaStream *mediaStream)
{
}
void HdmiSource::reset()
{
}

void HdmiSource::release()
{
}

bool HdmiSource::checkUrlSupport(const std::string& url)
{
    BME_DEBUG_ENTER();

    if (url.find(IMedia::HDMI_URI_PREFIX) != std::string::npos) {
        return true;
    }

    BME_DEBUG_EXIT();
    return false;
}

SourceConnector* HdmiSource::getConnector()
{
    return _connector;
}

uint32_t HdmiSource::setConnector(SourceConnector* connector)
{
    _connector = reinterpret_cast<SourceConnector*>(connector);
    return 0;
}


std::string HdmiSource::getType()
{
    return SOURCE_HDMI;
}

IMedia::StreamMetadata HdmiSource::getStreamMetadata()
{
    IMedia::StreamMetadata metadata;
    return metadata;
}

IMedia::ErrorType HdmiSource::seekTo(const uint32_t& milliseconds,
                        IMedia::PlaybackOperation playOperation,
                        IMedia::PlaybackSeekTime  playSeekTime)
{
    BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
    return IMedia::MEDIA_ERROR_NOT_SUPPORTED;
}

void HdmiSource::setPlaybackRate(const std::string& rate)
{
    BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
}

int HdmiSource::getPlaybackRate()
{
    BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
}

IMedia::PlaybackOperation HdmiSource::getPlaybackOperation()
{
    BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
}

uint64_t HdmiSource::getDuration()
{
    BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
}

const IMedia::TimeInfo HdmiSource::getTimeInfo()
{
    IMedia::TimeInfo timeinfo;
    return timeinfo;
}

void HdmiSource::onPrepared()
{
    notify(SourceEvents::Prepared);
}

void HdmiSource::onError(const IMedia::ErrorType& errorType)
{
    notify(SourceEvents::Error, errorType);
}

void HdmiSource::onCompletion()
{
    notify(SourceEvents::Completed);
}

Connector HdmiSource::connect(const ConnectSettings& settings)
{
    return NULL;
}

void HdmiSource::disconnect(const Connector& connector)
{
}

}  // namespace Broadcom
}
