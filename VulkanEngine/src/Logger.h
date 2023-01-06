/// \file Logger.h
#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <iostream>
#include <fstream>

class Logger 
{
public:
	Logger(SDL_Window* win = nullptr);

	~Logger();

	inline void ExceptionMSG(const char* msg)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", msg, _pWindow);
		exit(EXIT_FAILURE);
	}
	inline void ExceptionMSG(std::string& msg)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", msg.c_str(), _pWindow);
		exit(EXIT_FAILURE);
	}
	inline void ExceptionMSG(std::exception& msg)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", msg.what(), _pWindow);
		exit(EXIT_FAILURE);
	}
	inline void SetWindow(SDL_Window* win) 
	{
		_pWindow = win;
	}
private:
	SDL_Window* _pWindow;
	std::ofstream log,errlog;
	std::streambuf *OldLogBuf, *OldErrBuf,*OldOutBuf;
	const char* LogFileName   = "Log.txt";
	const char* ErrorFileName = "ErrLog.txt";
};