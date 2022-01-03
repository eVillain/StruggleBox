#include "Log.h"

#include "LogOutput.h"
#include "LogOutputSTD.h"
#include "StringUtil.h"
#include <sstream>
#include <iostream>
#include <memory>
#include <cstdarg>

LogLevel Log::_level = LogLevel::Log_Debug;
std::vector<LogOutput*> Log::_outputs = {};

void Log::SetLogLevel(const LogLevel level)
{
    _level = level;
}

void Log::Debug(const char * format, ...)
{
    if ( _level < LogLevel::Log_Debug ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    const std::string debugMessage = StringUtil::Format(format, args);
    va_end (args);
    for (auto output : _outputs)
    {
        output->Debug(debugMessage);
    }
}

void Log::Info(const char * format, ...)
{
    if ( _level < LogLevel::Log_Info ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    const std::string infoMessage = StringUtil::Format(format, args);
    va_end (args);
    for (auto output : _outputs)
    {
        output->Info(infoMessage);
    }
}

void Log::Warn(const char * format, ...)
{
    if ( _level < LogLevel::Log_Warn ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    const std::string warnMessage = StringUtil::Format(format, args);
    va_end (args);
    for (auto output : _outputs)
    {
        output->Warn(warnMessage);
    }
}

void Log::Error(const char * format, ...)
{
    if ( _level < LogLevel::Log_Error ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    const std::string errorMessage = StringUtil::Format(format, args);
    va_end (args);
    for (auto output : _outputs)
    {
        output->Error(errorMessage);
    }
}

