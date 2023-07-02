/// \file Logger.cpp
#include "Logger.h"

#include "Configure.h"

#include <exception>

namespace Multor
{
namespace Logging
{

Logger::Logger(SDL_Window* pWin) : pWindow_(pWin)
{
    sFileOutBuffer_.open(Multor::LogFileName, std::ios::out);
    if (!sFileOutBuffer_.is_open())
        throw std::runtime_error(
            "Logging system can't work without file ouput!");
}

Logger::~Logger()
{
    sFileOutBuffer_.close();
}

} // namespace Logging
} // namespace Multor