#ifndef LOG_OUTPUT_STD_H
#define LOG_OUTPUT_STD_H

#include "LogOutput.h"
#include <iostream>

class LogOutputSTD : public LogOutput
{
public:
    virtual void Debug(const std::string& message)
    {
        std::cout << "DEBUG: " << message << "\n";
    }
    
    virtual void Info(const std::string& message)
    {
        std::cout << "INFO: " << message << "\n";
    }
    
    virtual void Warn(const std::string& message)
    {
        std::cout << "WARN: " << message << "\n";
    }
    
    virtual void Error(const std::string& message)
    {
        std::cout << "ERROR: " << message << "\n";
    }
};

#endif /* LOG_OUTPUT_STD_H */
