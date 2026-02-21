/// \file time.cpp
#include "Time.h"

namespace Multor
{

double Chronometr::operator()()
{
    currentTime_ = std::chrono::high_resolution_clock::now();
    dt_ = std::chrono::duration<double, std::milli>(currentTime_ - lastTime_)
             .count() /
         std::milli::den;
    lastTime_ = currentTime_;
    return dt_;
}

double Chronometr::GetTime()
{
    return std::chrono::duration<double, std::milli>(
               std::chrono::high_resolution_clock::now() - startProgram_)
               .count() /
           milli_;
}

} // namespace Multor
