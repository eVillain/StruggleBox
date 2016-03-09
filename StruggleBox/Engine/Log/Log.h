//
//  Log.h
//  DungeonSmith
//
//  Created by eVillain on 11/10/15.
//  Copyright © 2015 The Drudgerist. All rights reserved.
//

#ifndef Log_h
#define Log_h

#include <string>
#include <map>

typedef enum : unsigned int
{
    Log_Silent = 0,     // No output from log
    Log_Error = 1,      // Only output errors
    Log_Warn = 2,       // Output warnings + errors
    Log_Info = 3,       // Output information logs + warnings + errors
    Log_Debug = 4,      // Output all messages including debug logs
} LogLevel;

class Log
{
public:
    static void SetLogLevel(const LogLevel level);
    
    static void Debug(const char * format, ...);
    static void Info(const char * format, ...);
    static void Warn(const char * format, ...);
    static void Error(const char * format, ...);
private:
    
    static const std::string FormatInput(const char * format, va_list args);
    
    static LogLevel _level;
};

#endif /* Log_h */
