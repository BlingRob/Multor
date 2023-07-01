/// \file Logger.h
#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <type_traits>

#include <SDL3/SDL.h>

namespace Multor
{
namespace Logging
{

/// @brief Base type logging message
struct LogTypes{};

/// @brief Information message type
struct Info : LogTypes
{
	static uint32_t const value = 0;
	static uint32_t const SDLvalue = SDL_MESSAGEBOX_INFORMATION;
	static inline char const name[] = "Info";
};

/// @brief Debug (warning) message type
struct Debug : LogTypes
{
	static uint32_t const value = 1;
	static uint32_t const SDLvalue = SDL_MESSAGEBOX_WARNING;
	static inline char const name[] = "Debug";
};

/// @brief Error message type
struct Error : LogTypes
{
	static uint32_t const value = 2;
	static uint32_t const SDLvalue = SDL_MESSAGEBOX_ERROR;
	static inline char const name[] = "Error";
};

/// @brief Base type logging output target
struct OutputTarget{};

/// @brief Output in file
struct File : OutputTarget{};
/// @brief Output in console
struct Console : OutputTarget{};
/// @brief Output in messagebox
struct Window : OutputTarget{};

/// @brief Logger class
class Logger 
{
public:
	Logger(SDL_Window* win = nullptr);

	~Logger();

	/// @brief Set association window, which can create message box
	/// @param win windows context
	void SetWindow(SDL_Window* win) 
	{
		_pWindow = win;
	}

	/// @brief Logging
	/// @tparam OutT target of output
	/// @tparam LogT logging type  
	/// @param msg message
	template<typename OutT, typename LogT, typename = std::enable_if_t<std::is_base_of_v<LogTypes, LogT>  && std::is_base_of_v<OutputTarget, OutT>>>
	void Log(const char* msg)
	{
		log<OutT, LogT>(msg);
	}

private:

	template<typename OutT, typename LogT>
	void log(const char* msg)
	{
	
		if constexpr(std::is_same_v<OutT, Window>)
		{
			if(_pWindow)
				SDL_ShowSimpleMessageBox(LogT::SDLvalue, LogT::name, msg, _pWindow);
			else
				log<File, LogT>(msg);
		}
		else if (std::is_same_v<OutT, Console>)
		{
			if(std::cout.rdbuf())
				std::cout << "[" << LogT::name << "] " << msg << std::endl;
			else
				log<File, LogT>(msg);
		}
		else if (std::is_same_v<OutT, File>)
		{
			if(_sFileOutBuffer.is_open())
				_sFileOutBuffer << "[" << LogT::name << "] " << msg << std::endl;
		}
	}

private:
	SDL_Window* _pWindow;
	std::ofstream _sFileOutBuffer;
};

} // namespace Logging
} // namespace Multor

#endif // LOGGER_H