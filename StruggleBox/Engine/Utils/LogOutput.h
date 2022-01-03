#ifndef LOG_OUTPUT_H
#define LOG_OUTPUT_H

#include <string>

class LogOutput
{
public:
    virtual void Debug(const std::string& message) = 0;
    virtual void Info(const std::string& message) = 0;
    virtual void Warn(const std::string& message) = 0;
    virtual void Error(const std::string& message) = 0;
};

#endif /* LOG_OUTPUT_H */
