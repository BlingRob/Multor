/// \file logger.h
#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <string_view>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "../configure.h"

#include "toml++/toml.hpp"
#include "quill/Backend.h"
#include "quill/Logger.h"
#include "quill/LogMacros.h"

namespace Multor
{
namespace Logging
{

/// @brief Logger class
class Logger
{
public:
    Logger(quill::Logger*);
    
    ~Logger() = default;

    quill::Logger* get();

private:
    
    quill::Logger* logger_;
};

class LoggerFactory
{
    public:

    LoggerFactory() = delete;
    LoggerFactory(LoggerFactory&&) = delete;
    LoggerFactory(const LoggerFactory&) = delete;

    static void Init(toml::table& config);

    static Logger& GetLogger(const std::string& logger_file = DEFAULT_LOG_FILE);

    private:

    static quill::Logger* createLogger(const std::string&);

    private:

    static inline std::filesystem::path logs_path_;

    static inline quill::BackendOptions backend_options_;

    static inline quill::LogLevel level_;

    static inline quill::PatternFormatterOptions pfo_;

    static inline std::unordered_map<std::string, std::unique_ptr<Logger>> loggers_;
};





} // namespace Logging
} // namespace Multor

#endif // LOGGER_H