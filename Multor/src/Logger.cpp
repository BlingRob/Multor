/// \file Logger.cpp
#include "Logger.h"

#include "Configure.h"

#include <exception>

namespace Multor
{
namespace Logging
{

Logger::Logger(SDL_Window* pWin) :_pWindow(pWin)
{
	_sFileOutBuffer.open(Multor::LogFileName, std::ios::out);
	if(!_sFileOutBuffer.is_open())
		throw std::runtime_error("Logging system can't work without file ouput!");
}

Logger::~Logger() 
{
	_sFileOutBuffer.close();
}

} // namespace Logging
} // namespace Multor