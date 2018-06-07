/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#ifndef BME_MEDIA_AUDIOVOLUMECHANGE_H_
#define BME_MEDIA_AUDIOVOLUMECHANGE_H_

#include <algorithm> // std::min()
#include <vector>
#include <mutex>

#include <math.h>

#include "ManualAnimate.h"


namespace Broadcom
{
namespace Media
{

struct BME_SO_EXPORT AudioVolumeBase
{
public:
    AudioVolumeBase(const char *name);
    virtual ~AudioVolumeBase();

protected:
    typedef std::vector<AudioVolumeBase *> list_t;
    static std::mutex lock;
    static list_t     list;
    static bool       ms12;
    const char       *name;

public:
    static float      master;

    virtual void  setVolume(float level) = 0;
    virtual float getVolume() const = 0;

    static void setMasterVolume(float level);

    // Use MS12 mixer to change the 'master' volume
    static void blurFastAll(unsigned int timeMs, float target, unsigned int profile);

    static unsigned int levelToNexus(float level);
    static float        nexusToLevel(unsigned int nexus);

    static bool isMs12()
    {
        return ms12;
    }

protected:
    virtual void applyMasterVolume() = 0; // Called with lock held

    virtual void blurFast(unsigned int timeMs, float target, unsigned int profile) = 0;
};

template<typename H, typename S>
struct BME_SO_EXPORT AudioOutput : public AudioVolumeBase
{
    typedef H Handle_t;
    typedef S Settings_t;

    // ignoreMaster prevents the output from being reduced - typically for text to speech
    AudioOutput(Handle_t handle, bool ignoreMaster, const char *name);
    Handle_t   handle;
    Settings_t settings;
    bool       ignoreMaster;
    float      volume;

    void  rawSetVolume(unsigned int level); // May or may not include the master level
    void  setVolume(float level) override;
    float getVolume() const override
    {
        return volume;
    }

    // Master volume has changed
    void applyMasterVolume() override;

    // Use MS12 mixer to change the volume
    // Cannot be paused or cancelled (implementation decision)
    void blurFast(unsigned int timeMs, float target, unsigned int profile) override;
};


// ===================================================================

// Base providing common functionality
// Needs a set() override
class BME_SO_EXPORT AudioVolumeChange : public ManualAnimate
{
protected:
    float from;
    float to;

    // Maps ChangeProfile to teh fucntion to use
    static const DistanceFunction profileFunction[5];

public:
    enum class ChangeProfile
    {
        Linear,        // Matches NEXUS_AudioFadeSettings::type
        CubicUp,       // Matches NEXUS_AudioFadeSettings::type
        CubicDown,     // Matches NEXUS_AudioFadeSettings::type
        QuadraticUp,   // Not supported by Nexus
        QuadraticDown  // Not supported by Nexus
    };

    AudioVolumeChange()
    {
    }
    ~AudioVolumeChange()
    {
        abort();
    }

protected:
    virtual void blur(unsigned int timeMs, float start, float end, ChangeProfile profile);
    // pause(), resume () and abort() available from the base class
};


// ===================================================================

class BME_SO_EXPORT AudioIndividualVolumeChange : public AudioVolumeChange
{
private:
    void set(double distance) override;

    AudioVolumeBase *output;

public:
    AudioIndividualVolumeChange(AudioVolumeBase *output);

    // pause(), resume () and abort() available from the base class
    void blur(unsigned int timeMs, float target, ChangeProfile profile);
};


// ===================================================================

class BME_SO_EXPORT AudioMasterVolumeChange : public AudioVolumeChange
{
private:
    // For manual change
    void set(double distance) override;

public:
    AudioMasterVolumeChange();

    // Starts both manual and Dolby mixer changes
    void blur(unsigned int timeMs, float target, ChangeProfile profile);
};


}
}


#endif //ifndef BME_MEDIA_AUDIOVOLUMECHANGE_H_
