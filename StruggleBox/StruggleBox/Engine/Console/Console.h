#pragma once

#include "ConsoleVar.h"
#include "ConsoleDefs.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

class CommandProcessor;

class Console
{
	friend class ConsoleDisplay;
public:
    static void Initialize();
    //static void Terminate();
    static void Print(const char * format, ... );
    static void PrintString(const std::string& text, Color col = COLOR_WHITE);
    static void AddMessage(const std::string& text, Color col);
    
    static void SaveLog();
    static void Process(const std::string& input);
    
    static void AddVar(ConsoleVar* newCVar, const std::string& varName);
    static void RemoveVar(const std::string& varName);

    template<typename T> static void AddVar( T& newVar, const std::string& varName ) {
        ConsoleVar* newCVar = new ConsoleVar( newVar );
        AddVar(newCVar, varName);
    };

    static void setUpdateCallback(std::function<void()> callback) { _updateCallback = callback; }
private:
    static std::map<std::string, ConsoleVar*> _cvars;
    static std::vector<ConsoleLine> _textLines;
    static std::function<void()> _updateCallback;

    static void Tokenize(const std::string& input, std::vector<std::string>& tokens);
};
