/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#include "ManualAnimate.h"


namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(ManualAnimate);

void ManualAnimate::pause()
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _pause = true;
    }
    _condition.notify_all();
}

void ManualAnimate::resume()
{
    if (!_thread.joinable())
        return;

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _pause = false;
    }
    _condition.notify_all();
}

void ManualAnimate::abort()
{
    if (!_thread.joinable()) {
        _pause = _exit = false;
        return;
    }

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _pause = false;
        _exit = true;
    }
    _condition.notify_all();
    _thread.join();
    _exit = false;
}

double ManualAnimate::linearDistance(double position)
{
    return position;
}
double ManualAnimate::quadraticDistanceUp(double position)
{
    return position * position;
}
double ManualAnimate::quadraticDistanceDown(double position)
{
    return 1.0 - quadraticDistanceUp(1.0 - position);
}
double ManualAnimate::cubicDistanceUp(double position)
{
    return position * position * position;
}
double ManualAnimate::cubicDistanceDown(double position)
{
    return 1.0 - cubicDistanceUp(1.0 - position);
}

void ManualAnimate::animate(unsigned int stepMilliSeconds, unsigned int steps, DistanceFunction distance)
{
    abort();
    _thread = std::thread(&ManualAnimate::main, this, stepMilliSeconds, steps, distance);
}

void ManualAnimate::main(unsigned int stepMilliSeconds, unsigned int steps, DistanceFunction distance)
{
    for (unsigned int i = 1; i <= steps; i++) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_condition.wait_for(lock, std::chrono::milliseconds(stepMilliSeconds), [&]() {return _pause || _exit;})) {
            // Wait to be unpaused
            _condition.wait(lock, [&]() {return !_pause || _exit;});
        }

        if (_exit)
            return;

        double x = (*distance)(double(i) / double(steps));
        set(x);
    }
}

}
}
