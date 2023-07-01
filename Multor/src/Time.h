/// \file Time.h

#pragma once
#ifndef TIME_H
#define TIME_H

#include <chrono>

namespace Multor
{

class Chronometr
{
	std::chrono::time_point<std::chrono::high_resolution_clock> startProgram, LastTime, CurrentTime;
	double dt;
	double milli = 1000.0;
public:
	Chronometr():dt(0.0)
	{
		CurrentTime =
		   LastTime = 
	   startProgram = std::chrono::high_resolution_clock::now();
	}
	/*Return time since start programme*/
	double GetTime();
	/*Return time since last call this operator*/
	double operator()();
};

} // namespace Multor

#endif // TIME_H