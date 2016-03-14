#ifndef CONSOLE_H
#define CONSOLE_H

#include "ConsoleVar.h"
#include "ConsoleDefs.h"
#include <vector>
#include <map>
#include <string>

class Locator;
class Renderer;
class CommandProcessor;
class TextInput;

class Console
{
public:
    static void Initialize(Locator& locator);
    static void Terminate();
    static void Print(const char * format, ... );
    static void PrintString(std::string text, Color col = COLOR_WHITE);
    static void AddMessage(std::string text, Color col);
    static void ToggleVisibility();
    static bool isVisible();
    static void Draw(double deltaTime);
    
    static void SaveLog();
    static void Process(std::string input);
    
    static void AddVar(ConsoleVar* newCVar,
                std::string varName);
    static void RemoveVar(std::string varName);

    template<typename T> static void AddVar( T& newVar, std::string varName ) {
        ConsoleVar* newCVar = new ConsoleVar( newVar );
        AddVar(newCVar, varName);
    };

private:
    static std::map<std::string, ConsoleVar*> _cvars;
    static std::vector<ConsoleLine> _textLines;
    static std::vector<std::shared_ptr<Label>> _textLabels;
    static std::shared_ptr<TextInput> _textInput;
    static bool _visible;

    static Locator* _locator;
    
    static void Refresh();
    static void Show();
    static void Hide();
    static void OnTextInput(const std::string& input);
    static void Tokenize(const std::string& input,
                  std::vector<std::string>& tokens);
};

#endif /* CONSOLE_H */
