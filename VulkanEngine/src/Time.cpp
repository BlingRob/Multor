/// \file Time.cpp
#include "Time.h"

double Chronometr::operator()()
{
	CurrentTime = std::chrono::high_resolution_clock::now();
	dt = std::chrono::duration<double, std::milli>(CurrentTime - LastTime).count() / std::milli::den;
	LastTime = CurrentTime;
	return dt;
}

double Chronometr::GetTime()
{
	return std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - startProgram).count() / milli;
}