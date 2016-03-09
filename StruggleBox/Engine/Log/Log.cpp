#include "Log.h"
#include "Console.h"
#include "StringUtil.h"
#include <sstream>
#include <iostream>

LogLevel Log::_level = Log_Warn;

//static const Color LOG_COLOR_ERROR = COLOR_RED;
//static const Color LOG_COLOR_WARN = COLOR_YELLOW;
//static const Color LOG_COLOR_DEBUG = COLOR_GREEN;
//static const Color LOG_COLOR_INFO = COLOR_WHITE;

void Log::SetLogLevel(const LogLevel level)
{
    _level = level;
}


void Log::Debug( const char * format, ... )
{
    if ( _level < Log_Debug ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    std::string debugMessage = StringUtil::Format(format, args);
    va_end (args);
    std::cout << "DEBUG: " << debugMessage << "\n";
    //    Console::PrintString(debugMessage, LOG_COLOR_DEBUG);
}


void Log::Info( const char * format, ... )
{
    if ( _level < Log_Info ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    std::string infoMessage = StringUtil::Format(format, args);
    va_end (args);
    std::cout << "INFO: " << infoMessage << "\n";
    //    Console::PrintString(debugMessage, LOG_COLOR_INFO);
}

void Log::Warn( const char * format, ... )
{
    if ( _level < Log_Warn ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    std::string warnMessage = StringUtil::Format(format, args);
    va_end (args);
    std::cout << "WARN: " << warnMessage << "\n";
    //    Console::PrintString(warnMessage, LOG_COLOR_WARN);
}

void Log::Error( const char * format, ... )
{
    if ( _level < Log_Error ) return;
    // Grab arguments
    va_list args;
    va_start (args, format);
    std::string errorMessage = StringUtil::Format(format, args);
    va_end (args);
    std::cout << "ERROR: " << errorMessage << "\n";
//    Console::PrintString(errorMessage, LOG_COLOR_ERROR);
}
