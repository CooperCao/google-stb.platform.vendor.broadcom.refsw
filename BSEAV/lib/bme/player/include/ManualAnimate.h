/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#ifndef BME_MANUALANIMATE_H_
#define BME_MANUALANIMATE_H_

#include "Debug.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>


namespace Broadcom
{
namespace Media
{
class BME_SO_EXPORT ManualAnimate
{
private:
    std::thread             _thread;
    std::mutex              _mutex;
    std::condition_variable _condition;
    bool                    _pause;
    bool                    _exit;

protected:
    virtual void set(double distance) = 0;
    static double blend(double start, double end, double distance)
    {
        return start*(1.0 - distance) + end*distance;
    }

public:
    // Distance functions to allow for different transition profiles
    // position: 0.0 .. 1.0
    typedef double (* DistanceFunction)(double position);
    static double linearDistance(double position);
    static double quadraticDistanceUp(double position);
    static double quadraticDistanceDown(double position);
    static double cubicDistanceUp(double position);
    static double cubicDistanceDown(double position);

    void animate(unsigned int stepMilliSeconds, unsigned int steps, DistanceFunction distance);
    void pause();
    void resume();
    void abort();

    ManualAnimate() : _pause(false), _exit(false)
    {
    }

    virtual ~ManualAnimate()
    {
        abort();
    }

private:
    void main(unsigned int stepMilliSeconds, unsigned int steps, DistanceFunction distance);
};

}
}


#endif // ifndef BME_MANUALANIMATE_H_