/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#include "MonitorSource.h"
#include "nexus_message.h"
#include "nexus_pid_channel.h"
#include "berr.h"
#include "bkni.h"

namespace Broadcom {
namespace Media {

TRLS_DBG_MODULE(MonitorSource);

typedef struct MonitorSourceContext {
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_MessageHandle msgHandle;
} MonitorSourceContext;

typedef struct {
    NEXUS_MessageFormat nexus;
    IMedia::TransportSegmentFormat settop;
} MsgTypeEntry;

NEXUS_MessageFormat convertTransportSegmentFormatType(const IMedia::TransportSegmentFormat&
                                                                               media_value);

MonitorSource::MonitorSource()
{
    _ctx = new MonitorSourceContext;
}

MonitorSource::~MonitorSource()
{
    delete _ctx;
}

void MonitorSource::setMonitorSourceConfig(MonitorSourceConfig config)
{
    _cfg = config;
}

static void messageCallback(void *context, int pid)
{
    BME_DEBUG_ENTER();
    MonitorSource* obj;
    if (context == NULL) {
        return;
    }
    obj = static_cast<MonitorSource*>(context);
    obj->onMessageCallback();
    BME_DEBUG_EXIT();
}

IMedia::ErrorType MonitorSource::start()
{
    NEXUS_MessageSettings openSettings;
    NEXUS_MessageStartSettings startSettings;
    ConnectSettings connectSettings;
    BaseSource* src;
    src = (BaseSource*)_cfg.source;
    connectSettings.streamId = _cfg.pid;
    _ctx->pidChannel = static_cast<NEXUS_PidChannelHandle>(src->connect(connectSettings));
    if (_ctx->pidChannel == NULL) {
        return IMedia::ErrorType::MEDIA_ERROR_UNKNOWN;
    }
    NEXUS_Message_GetDefaultSettings(&openSettings);
    openSettings.dataReady.callback = messageCallback;
    openSettings.dataReady.context = this;
    openSettings.dataReady.param = _cfg.pid;
    if (_cfg.format == IMedia::TransportSegmentFormat::Ts) {
        openSettings.maxContiguousMessageSize = 0;
    }
    _ctx->msgHandle = NEXUS_Message_Open(&openSettings);
    NEXUS_Message_GetDefaultStartSettings(_ctx->msgHandle, &startSettings);
    startSettings.pidChannel = _ctx->pidChannel;
    startSettings.format = convertTransportSegmentFormatType(_cfg.format);
    NEXUS_Message_Start(_ctx->msgHandle, &startSettings);
    return IMedia::ErrorType::MEDIA_SUCCESS;
}

IMedia::ErrorType MonitorSource::stop()
{
    BaseSource* src;
    NEXUS_Message_Close(_ctx->msgHandle);
    src = (BaseSource*)_cfg.source;
    src->disconnect(static_cast<void*>(_ctx->pidChannel));
    return IMedia::ErrorType::MEDIA_SUCCESS;
}

void MonitorSource::onMessageCallback()
{
    const void* buffer1;
    const void* buffer2;
    void* notifyAppBuffer;
    size_t size1 = 0;
    size_t size2 = 0;
    size_t notifyAppSize = 0;
    if (_cfg.format == IMedia::TransportSegmentFormat::Ts) {
        NEXUS_Message_GetBufferWithWrap(_ctx->msgHandle, &buffer1, &size1, &buffer2, &size2);
        if (size1 && size2) {
            notifyAppBuffer =  malloc(size1 + size2);
            if (buffer1) {
                BKNI_Memcpy(notifyAppBuffer, buffer1, size1);
            }
            if (buffer2) {
                BKNI_Memcpy(((uint8_t*)(notifyAppBuffer) + size1), buffer2, size2);
            }
            notifyAppSize = size1 + size2;
            notifyAppSize = notifyAppSize - (notifyAppSize % 188);
        } else if (size1) {
            notifyAppSize = size1 - (size1 % 188);
            notifyAppBuffer = (void*)buffer1;
        } else if (size2) {
            notifyAppSize = size2 - (size2 % 188);
            notifyAppBuffer = (void*)buffer2;
        } else {
            notifyAppSize = 0;
            notifyAppBuffer = NULL;
        }
    } else {
        NEXUS_Message_GetBuffer(_ctx->msgHandle, (const void**)&notifyAppBuffer,
                                                                &notifyAppSize);
    }
    if (notifyAppSize) {
        notify(MonitorSourceEvents::TransportStreamPackets, _cfg,
                                    notifyAppBuffer, notifyAppSize);
    }
    NEXUS_Message_ReadComplete(_ctx->msgHandle, notifyAppSize);
    if (size1 && size2) {
        free(notifyAppBuffer);
    }
}

MsgTypeEntry msgTypes[] = {
    {NEXUS_MessageFormat_ePsi, IMedia::TransportSegmentFormat::Psi},
    {NEXUS_MessageFormat_ePes, IMedia::TransportSegmentFormat::Pes},
    {NEXUS_MessageFormat_ePesSaveAll, IMedia::TransportSegmentFormat::PesSaveAll},
    {NEXUS_MessageFormat_eTs, IMedia::TransportSegmentFormat::Ts},
    {NEXUS_MessageFormat_eMax, IMedia::TransportSegmentFormat::Max}
};

#define CONVERT(g_struct) \
    unsigned i; \
    for (i = 0; i < sizeof(g_struct)/sizeof(g_struct[0]); i++) { \
        if (g_struct[i].settop == media_value) { \
            return g_struct[i].nexus; \
        } \
    } \
    printf("unable to find value %d in %s\n", media_value, #g_struct); \
    return g_struct[0].nexus

NEXUS_MessageFormat convertTransportSegmentFormatType(const IMedia::TransportSegmentFormat& media_value)
{
    CONVERT(msgTypes);
}

}  // namespace Media
}  // namespace Broadcom
