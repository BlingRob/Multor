/// \file configure.h
/// \brief Configuration file

#pragma once
#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <stdint.h>

namespace Multor
{
inline const char* CFG_DEFAULT_FILE = "config.toml";

inline const char* DEFAULT_LOG_FILE = "application.log";

inline const char* DEFAULT_LOG_PATH = ".";

inline std::size_t DEFAULT_LOG_LEVEL{0};
};

#endif // CONFIGURE_H