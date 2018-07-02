/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#include <cmath>
#include <algorithm>
#include "AudioVolumeChange.h"
#include "nxclient.h"
#include "nexus_simple_audio_playback.h" // NEXUS_SimpleAudioPlaybackHandle
#include "nexus_simple_audio_decoder.h"  // NEXUS_SimpleAudioDecoderHandle


namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(AudioVolumeChange);


// ===================================================================

NxClient_AudioStatus audioStatus;

static bool ms12Enabled()
{
    BME_CHECK(NxClient_Join(NULL));
    NxClient_GetAudioStatus(&audioStatus);
    if (audioStatus.dolbySupport.mixer)
    {
        NxClient_AudioProcessingSettings settings;
        NxClient_GetAudioProcessingSettings(&settings);
        settings.dolby.ddre.fixedEncoderFormat = true;
        BME_CHECK(NxClient_SetAudioProcessingSettings(&settings));
    }
    NxClient_Uninit();
    return audioStatus.dolbySupport.mixer;
}

std::mutex              AudioVolumeBase::lock;
AudioVolumeBase::list_t AudioVolumeBase::list;
float                   AudioVolumeBase::master = 1.0f;
bool                    AudioVolumeBase::ms12   = ms12Enabled();

AudioVolumeBase::AudioVolumeBase(const char *name) : name(name)
{
    std::lock_guard<std::mutex> list_lock(lock);
    list.push_back(this);
}

AudioVolumeBase::~AudioVolumeBase()
{
    std::lock_guard<std::mutex> list_lock(lock);
    for (list_t::iterator it = list.begin(); it != list.end(); ++it)
        if (*it == this) {
            list.erase(it);
            return;
        }
}

void AudioVolumeBase::setMasterVolume(float level)
{
    std::lock_guard<std::mutex> list_lock(lock);
    master = level;
    for (list_t::iterator it = list.begin(); it != list.end(); ++it)
        (*it)->applyMasterVolume();
}

void AudioVolumeBase::blurFastAll(unsigned int timeMs, float target, unsigned int profile)
{
    std::lock_guard<std::mutex> list_lock(lock);
    for (list_t::iterator it = list.begin(); it != list.end(); ++it)
        (*it)->blurFast(timeMs, target, profile);
}

unsigned int AudioVolumeBase::levelToNexus(float level)
{
    int nexus = static_cast<int>(std::lround(level * float(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL)));
    return static_cast<unsigned int>(std::max(
        std::min(nexus, NEXUS_AUDIO_VOLUME_LINEAR_MAX), NEXUS_AUDIO_VOLUME_LINEAR_MIN));
}

float        AudioVolumeBase::nexusToLevel(unsigned int nexus)
{
    return float(nexus) / float(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL);
}


// ===================================================================

// NEXUS_SimpleAudioPlayback
typedef AudioOutput<NEXUS_SimpleAudioPlaybackHandle, NEXUS_SimpleAudioPlaybackSettings> AudioOutputPlayback;

template<>
void AudioOutputPlayback::rawSetVolume(unsigned int level)
{
    if (level == settings.rightVolume)
        return;
    settings.rightVolume = settings.leftVolume = level;
    NEXUS_SimpleAudioPlayback_SetSettings(handle, &settings);
}
template<>
void AudioOutputPlayback::applyMasterVolume()
{
    if (ignoreMaster)
        return;
    unsigned int nexus = levelToNexus(volume*master);
    rawSetVolume(nexus);
}
template<>
void AudioOutputPlayback::setVolume(float level)
{
    std::lock_guard<std::mutex> list_lock(lock);
    volume = level;
    unsigned int nexus = levelToNexus(volume*master);
    rawSetVolume(nexus);
}
template<>
void AudioOutputPlayback::blurFast(unsigned int timeMs, float target, unsigned int profile)
{
    // Don't have the capability, so ignore
}
template<>
AudioOutputPlayback::AudioOutput(Handle_t handle, bool ignoreMaster, const char *name)
: AudioVolumeBase(name), handle(handle), ignoreMaster(ignoreMaster)
{
    NEXUS_SimpleAudioPlayback_GetSettings(handle, &settings);
    setVolume(1.0f);
}


// NEXUS_SimpleAudioDecoder
typedef AudioOutput<NEXUS_SimpleAudioDecoderHandle, NEXUS_SimpleAudioDecoderSettings> AudioOutputDecoder;

template<>
void AudioOutputDecoder::rawSetVolume(unsigned int level)
{
    if (level == settings.primary.volumeMatrix[0][0]) {
        return;
    }
    for (unsigned int i = 0; i< NEXUS_AudioChannel_eMax; ++i)
        settings.primary.volumeMatrix[i][i] = level;
    NEXUS_SimpleAudioDecoder_SetSettings(handle, &settings);
}
template<>
void AudioOutputDecoder::applyMasterVolume()
{
    if (ms12 || ignoreMaster)
        return; // Master volume changes via blurFast() (MS12 mixer)

    unsigned int nexus = levelToNexus(volume*master);
    rawSetVolume(nexus);
}
template<>
void AudioOutputDecoder::setVolume(float level)
{
    std::lock_guard<std::mutex> list_lock(lock);
    volume = level;
    // Ignore the master volume level for MS12, as we use blurFast() instead
    unsigned int nexus = levelToNexus(ms12 || ignoreMaster ? volume : volume*master);
    rawSetVolume(nexus);
}
template<>
void AudioOutputDecoder::blurFast(unsigned int timeMs, float target, unsigned int profile)
{
    if (ignoreMaster)
        return; // Not subject to the master volume control

    NEXUS_SimpleAudioDecoderProcessorSettings &mixer =
        settings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary];
    mixer.fade.connected = true;
    mixer.fade.settings.level    = std::roundl(target * 100.0f);
    mixer.fade.settings.duration = timeMs;
    mixer.fade.settings.type     = profile;
    NEXUS_SimpleAudioDecoder_SetSettings(handle, &settings);
}
template<>
AudioOutputDecoder::AudioOutput(Handle_t handle, bool ignoreMaster, const char *name)
: AudioVolumeBase(name), handle(handle), ignoreMaster(ignoreMaster)
{
    NEXUS_SimpleAudioDecoder_GetSettings(handle, &settings);
    setVolume(1.0f);
}


// ===================================================================

const AudioVolumeChange::DistanceFunction AudioVolumeChange::profileFunction[5] =
{
    linearDistance,
    quadraticDistanceUp,
    quadraticDistanceDown,
    cubicDistanceUp,
    cubicDistanceDown
};

void AudioVolumeChange::blur(unsigned int timeMs, float start, float end, ChangeProfile profile)
{
    from = start;
    to   = end;
    unsigned int steps = std::max(timeMs / 30, 4U);
    animate(timeMs / steps, steps, profileFunction[(unsigned int) profile]);
}


// ===================================================================

AudioIndividualVolumeChange::AudioIndividualVolumeChange(AudioVolumeBase *output) : output(output)
{
}

void AudioIndividualVolumeChange::set(double distance)
{
    float current = blend(from, to, distance);
    output->setVolume(current);
}

void AudioIndividualVolumeChange::blur(unsigned int timeMs, float target, ChangeProfile profile)
{
    AudioVolumeChange::blur(timeMs, output->getVolume(), target, profile);
}


// ===================================================================

AudioMasterVolumeChange::AudioMasterVolumeChange()
{
}

void AudioMasterVolumeChange::set(double distance)
{
    float current = blend(from, to, distance);
    AudioVolumeBase::setMasterVolume(current);
}

void AudioMasterVolumeChange::blur(unsigned int timeMs, float target, ChangeProfile profile)
{
    // Start the manual change
    AudioVolumeChange::blur(timeMs, AudioVolumeBase::master, target, profile);

    if (!AudioVolumeBase::isMs12())
        return; // Nothing left to do if there's no Dolby mixer

    // Nexus doesn't support quadratic profiles, so we substitute cubic ones
    // In practice, the master volume is changed quickly, so the difference
    // between the profiles is neglible
    unsigned int fastProfile = (unsigned int) profile;
    if (fastProfile > (unsigned int) ChangeProfile::CubicDown)
        fastProfile -= (unsigned int) ChangeProfile::QuadraticDown - (unsigned int) ChangeProfile::CubicDown;

    // Start the Dolby mixer changes (lower levels check for it being enabled)
    AudioVolumeBase::blurFastAll(timeMs, target, fastProfile);
}

}
}
