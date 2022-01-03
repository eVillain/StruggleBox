#pragma once

#include "LogOutput.h"
#include "Console.h"

class LogOutputConsole : public LogOutput
{
public:
    virtual void Debug(const std::string& message)
    {
        Console::Print("DEBUG: %s", message.c_str());
    }
    
    virtual void Info(const std::string& message)
    {
        Console::Print("INFO: %s", message.c_str());
    }
    
    virtual void Warn(const std::string& message)
    {
        Console::Print("WARN: %s", message.c_str());
    }
    
    virtual void Error(const std::string& message)
    {
        Console::Print("ERROR: %s", message.c_str());
    }
};

