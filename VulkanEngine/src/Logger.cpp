/// \file Logger.cpp
#include "Logger.h"

Logger::Logger(SDL_Window* win) :_pWindow(win)
{
	log.open(LogFileName, std::ios::out);
	errlog.open(ErrorFileName, std::ios::out);
	OldOutBuf = std::cout.rdbuf(log.rdbuf());
	OldLogBuf = std::clog.rdbuf(log.rdbuf());
	OldErrBuf = std::cerr.rdbuf(log.rdbuf());
}

Logger::~Logger() 
{
	std::cout.rdbuf(OldOutBuf);
	std::clog.rdbuf(OldLogBuf);
	std::cerr.rdbuf(OldErrBuf);
}