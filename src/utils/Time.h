/// \file Time.h

#pragma once
#ifndef TIME_H
#define TIME_H

#include <chrono>

namespace Multor
{

class Chronometr
{
    std::chrono::time_point<std::chrono::high_resolution_clock> startProgram_,
        lastTime_, currentTime_;
    double dt_;
    double milli_ = 1000.0;

public:
    Chronometr() : dt_(0.0)
    {
        currentTime_ = lastTime_ = startProgram_ =
            std::chrono::high_resolution_clock::now();
    }
    /*Return time since start programme*/
    double GetTime();
    /*Return time since last call this operator*/
    double operator()();
};

} // namespace Multor

#endif // TIME_H
