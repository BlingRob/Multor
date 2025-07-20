/// \file Logger.cpp

#include "logger.h"

#include "quill/Frontend.h"
#include "quill/sinks/FileSink.h"

#include <exception>
#include <filesystem>

namespace Multor
{
namespace Logging
{

Logger::Logger(quill::Logger* logger) : logger_(logger)
{

}

quill::Logger* Logger::get()
{
    return logger_;
}

void LoggerFactory::Init(toml::table& config)
{
    quill::Backend::start(backend_options_);

    quill::PatternFormatterOptions pfo;
    pfo.format_pattern     = "%(time) [%(thread_id)] %(source_location:<28) "
                             "LOG_%(log_level:<9) %(message)";
    pfo.timestamp_pattern  = ("%H:%M:%S.%Qns");
    pfo.timestamp_timezone = quill::Timezone::GmtTime;

    switch (config["logging"]["log_level"].value_or(DEFAULT_LOG_LEVEL))
        {
            case 0:
                level_ = quill::LogLevel::Critical;
                break;
            case 1:
                level_ = quill::LogLevel::Error;
                break;
            case 2:
                level_ = quill::LogLevel::Warning;
                break;
            case 3:
                level_ = quill::LogLevel::Info;
                break;
            case 4:
                level_ = quill::LogLevel::Debug;
                break;
            case 5:
            default:
                level_ = quill::LogLevel::TraceL3;
                break;
        }
    
    logs_path_ = config["logging"]["log_dir"].value_or(DEFAULT_LOG_PATH);
    std::filesystem::create_directories(logs_path_);
}

Logger& LoggerFactory::GetLogger(const std::string& logger_file)
{
    if (auto it {loggers_.find(logger_file)}; it != loggers_.end())
        {
            return *it->second;
        }

    loggers_[logger_file] = std::make_unique<Logger>(createLogger(logger_file));

    return *loggers_[logger_file];
}

quill::Logger* LoggerFactory::createLogger(const std::string& logger_file)
{
    std::filesystem::path full_path(logs_path_ / std::filesystem::path(logger_file));
    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
        full_path.string(),
        []() {
            quill::FileSinkConfig cfg;
            cfg.set_open_mode('w');
            cfg.set_filename_append_option(
                quill::FilenameAppendOption::StartDateTime);
            return cfg;
        }(),
        quill::FileEventNotifier {});

    auto logger = quill::Frontend::create_or_get_logger(
        logger_file, std::move(file_sink), pfo_);

    logger->set_log_level(level_);

#ifndef NDEBUG
    logger->set_immediate_flush(1u);
#endif

    return logger;
}

} // namespace Logging
} // namespace Multor