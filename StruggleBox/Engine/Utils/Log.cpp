#include "Log.h"
#include "LogOutput.h"
#include "StringUtil.h"
#include <sstream>
#include <iostream>
#include <memory>
#include <cstdarg>

// All messages are shown by default, override in application as needed
LogLevel Log::_level = Log_Debug;
std::vector<std::shared_ptr<LogOutput>> Log::_outputs;

void Log::SetLogLevel(const LogLevel level)
{
    _level = level;
}

void Log::Debug(const char * format, ...)
{
    if ( _level < Log_Debug ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    std::string debugMessage = StringUtil::Format(format, args);
    va_end (args);
    for (auto output : _outputs)
    {
        output->Debug(debugMessage);
    }
}

void Log::Info(const char * format, ...)
{
    if ( _level < Log_Info ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    std::string infoMessage = StringUtil::Format(format, args);
    va_end (args);
    for (auto output : _outputs)
    {
        output->Debug(infoMessage);
    }
}

void Log::Warn(const char * format, ...)
{
    if ( _level < Log_Warn ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    std::string warnMessage = StringUtil::Format(format, args);
    va_end (args);
    for (auto output : _outputs)
    {
        output->Debug(warnMessage);
    }
}

void Log::Error(const char * format, ...)
{
    if ( _level < Log_Error ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    std::string errorMessage = StringUtil::Format(format, args);
    va_end (args);
    for (auto output : _outputs)
    {
        output->Debug(errorMessage);
    }
}

void Log::AttachOutput(const std::shared_ptr<LogOutput> output)
{
    _outputs.push_back(output);
}
