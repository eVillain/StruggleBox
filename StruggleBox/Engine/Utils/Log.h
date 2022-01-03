#ifndef LOG_H
#define LOG_H

#include <string>
#include <vector>
#include <memory>

enum class LogLevel
{
    Log_Silent = 0,     // No output from log
    Log_Error = 1,      // Only output errors
    Log_Warn = 2,       // Output warnings + errors
    Log_Info = 3,       // Output information logs + warnings + errors
    Log_Debug = 4,      // Output all messages including debug logs
};

class LogOutput;

class Log
{
public:
    static void SetLogLevel(const LogLevel level);
    
    static void Debug(const char* format, ...);
    static void Info(const char* format, ...);
    static void Warn(const char* format, ...);
    static void Error(const char* format, ...);

    template<typename T>
    static void AttachOutput(T* output)
    {
        _outputs.push_back(output);
    }
private:    
    static LogLevel _level;
    static std::vector<LogOutput*> _outputs;
};

#endif /* LOG_H */
