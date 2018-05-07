/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef LIB_MEDIA_MONITORSOURCE_H_
#define LIB_MEDIA_MONITORSOURCE_H_

#include <string>
#include "Media.h"
#include "BaseSource.h"

namespace Broadcom {
namespace Media {

enum class MonitorSourceEvents {
    TransportStreamPackets
};

struct MonitorSourceContext;

class MonitorSourceConfig
{
public:
    uint16_t pid;
    IMedia::TransportSegmentFormat format;
    size_t source;
    uint16_t userData;
    bool operator<(const MonitorSourceConfig& cfg) const
    {
        if (cfg.source == this->source) {
            if (cfg.format == this->format) {
                return (cfg.pid < this->pid);
            }
            return (cfg.format < this->format);
        }
        return (cfg.source < this->source);
    }
};

class BME_SO_EXPORT MonitorSource
    : public Observable<MonitorSourceEvents>
{
public:
    MonitorSource();
    ~MonitorSource();
    void setMonitorSourceConfig(MonitorSourceConfig config);
    IMedia::ErrorType start();
    IMedia::ErrorType stop();
    void onMessageCallback();

private:
    MonitorSourceConfig _cfg;
    MonitorSourceContext* _ctx;
};

class MonitorSourceListener
{
public:
    MonitorSourceListener() {}
    virtual ~MonitorSourceListener() {}

    void observe(MonitorSource* ms)
    {
        std::function<void(MonitorSourceConfig monitorCfg,
                           void* data, uint16_t dataLen)> onTransportMessage =
                std::bind(&MonitorSourceListener::onTransportMessage, this, _1, _2, _3);
        ms->addListener(MonitorSourceEvents::TransportStreamPackets, onTransportMessage);
    }

private:
    virtual void onTransportMessage(MonitorSourceConfig monitorCfg, void* data, uint16_t dataLen)
    {
    }
};
}  // namespace Media
}  // namespace Broadcom
#endif  // LIB_MEDIA_MONITORSOURCE_H_
