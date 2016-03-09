#include "Console.h"
#include "Locator.h"
#include "Renderer.h"
#include "CommandProcessor.h"
#include "Options.h"
#include "TextManager.h"
#include "FileUtil.h"
#include "Timer.h"
#include "UITextInput.h"
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <fstream>

std::map<std::string, ConsoleVar*> Console::_cvars;
std::vector<ConsoleLine> Console::_textLines;
bool Console::_visible;
UITextInputSCB* Console::_textWidget;
Locator* Console::_locator = nullptr;

// This thing is just there to force the templates to be compiled
inline void VarLoadGuard( void )
{
    bool b = true;
    int i = 1;
    float f = 1.5f;
    std::string s = "Yep";
    Console::AddVar(b, "");
    Console::AddVar(i, "");
    Console::AddVar(f, "");
    Console::AddVar(s, "");
};

void Console::Initialize(Locator& locator)
{
    _locator = &locator;
    PrintString("Console initialized, good times ahead", COLOR_GREEN);
    CommandProcessor::AddCommand("toggleConsole", Command<>(ToggleVisibility));
    CommandProcessor::AddCommand("vars", Command<>([&](){
        Print("Print vars (%l)", _cvars.size());
        std::map<std::string, ConsoleVar*>::const_iterator it;
        for ( it = _cvars.begin(); it != _cvars.end(); it++ ) {
            PrintString( it->first, COLOR_GREEN );
        }
    }));
}

bool Console::isVisible() { return _visible; };

void Console::Process( std::string command )
{
    // Try to tokenize input text
    std::vector<std::string> tokens;
    Tokenize(command, tokens);
    bool wasCVar = false;
    if ( tokens.size() == 1 ) { // Check if we just want to display the value of a ConsoleVar
        std::map<std::string, ConsoleVar*>::iterator it = _cvars.find( tokens[0] );
        if ( it != _cvars.end() ) {
            Print("ConsoleVar: %s = %s", tokens[0].c_str(), (*it->second).GetString().c_str() );
            wasCVar = true;
        }
    } else {
        // Check for known commands
        if ( tokens[0] == "set" ) {
            bool setNewValue = false;
            if ( tokens.size() == 3 ) {
                std::map<std::string, ConsoleVar*>::iterator it = _cvars.find( tokens[1] );
                if ( it != _cvars.end() && !tokens[2].empty() ) {    // Found cVar to set new value
                    if ( it->second->IsType<bool>() ) {
                        bool & cv = it->second->as<bool>();
                        if ( !is_number(tokens[2]) ) {
                            if ( tokens[2] == "false" ) {
                                cv = false;
                            } else if ( tokens[2] == "true" ) {
                                cv = true;
                            }
                            setNewValue = true;
                        } else {
                            int val = stoi(tokens[2]);
                            cv = val;
                            setNewValue = true;
                        }
                    } else if ( it->second->IsType<int>() ) {
                        int val = stoi(tokens[2]);
                        int & cv = it->second->as<int>();
                        cv = val;
                        setNewValue = true;
                    } else if ( it->second->IsType<float>() ) {
                        float val = stof(tokens[2]);
                        float & cv = it->second->as<float>();
                        cv = val;
                        setNewValue = true;
                    } else if ( it->second->IsType<double>() ) {
                        double val = stod(tokens[2]);
                        double & cv = it->second->as<double>();
                        cv = val;
                        setNewValue = true;
                    } else if ( it->second->IsType<std::string>() ) {
                        std::string & cv = it->second->as<std::string>();
                        cv = tokens[2];
                        setNewValue = true;
                    }
                } else {
                    Print("Unknown cvar: %s", tokens[1].c_str());
                }
            }
            if ( setNewValue == false ) {
                Print("Usage: set variable value");
            } else {
                Print("%s set to %s", tokens[1].c_str(), tokens[2].c_str());
            }
            wasCVar = true;
        }
    }
    if ( !wasCVar ) {   // Pass input to command processor
        CommandProcessor::Buffer(command);
    }
    _textWidget->Clear();
}
// The mother lode! Parses arguments almost like a real printf()
// True monkey coding at its best - throw shit at it until something sticks...
void Console::Print( const char * str, ... ) {
    // Grab arguments
    va_list args;
    va_start(args, str);
    std::ostringstream output;
    // Parse arguments
    for (unsigned int i = 0; i < strlen(str); i++) {
        // If we need to format the next char
        if (str[i] == '%' && str[i+1] != '%' && i+1 < strlen(str)) {
            // Switch the next character
            switch (str[i+1]) {
                case 's': {
                    char *temp = va_arg (args, char*);
                    output << temp;
                }
                    break;
                case 'i':
                case 'd': {
                    int temp = va_arg (args, int);
                    output << temp;
                }
                    break;
                case 'u': {
                    unsigned int temp = va_arg (args, unsigned int);
                    output << temp;
                }
                    break;
                case 'f': {
                    double temp = va_arg (args, double);
                    output << temp;
                }
                    break;
                case 'l': {
                    long temp = va_arg (args, long);
                    output << temp;
                }
                    break;
                case 'b': {
                    int var = va_arg (args, int);
                    std::string temp = var ? "True" : "False";
                    output << temp;
                }
                    break;
                default: {
                    output << str[i];
                }
                    break;
            }
            // Skip over the next character
            i++;
        }
        else if (str[i] == '%' && str[i+1] == '%' && i+1 < strlen(str) ) {
            output << str[i];
            // Skip over one percentage character
            i ++;
        }
        else if (str[i] == '\\' && str[i+1] == 'n' && i+1 < strlen(str) ) {
            // Skip over newline character
            i ++;
        }
        else { output << str[i]; }
    }
    va_end(args);
    
    // Finally we have a string to dump into the console output
    PrintString( output.str() );
}
void Console::PrintString( std::string text, Color col ) {
    // No parsing necessary just add timestamp
    std::string timeS = Timer::TimeStamp();
    timeS.append(" - ");
    timeS.append( text );
    AddMessage( timeS , col );
}

void Console::AddMessage(std::string text,
                         Color col)
{
    int newLabelID = -1;
    ConsoleLine newLine = { text, newLabelID, col, 0.0 };
    _textLines.push_back(newLine);
    Refresh();
}

void Console::ToggleVisibility()
{
    if ( _visible ) { Hide(); }
    else { Show(); }
}

void Console::Draw(double deltaTime)
{
    if (!_visible) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    int winWidth = _locator->Get<Options>()->GetOptionDataPtr<int>("r_resolutionX");
    int winHeight = _locator->Get<Options>()->GetOptionDataPtr<int>("r_resolutionY");
    // Draw background box
    Renderer* renderer = _locator->Get<Renderer>();
    renderer->Draw2DRect( glm::vec2(0.0f, (winHeight/4.0f)-1.0f), winWidth-1.0f, winHeight/2.0f,
                         COLOR_NONE, LAColor((GLfloat)0.1f,(GLfloat)0.5f), CONSOLE_BG_DEPTH );
}

void Console::SaveLog()
{
    std::string logPath = FileUtil::GetPath().append("Console.log");
    std::ofstream file( logPath.c_str() );
    for(unsigned int i=0; i< _textLines.size(); i++) {
        file << _textLines[i].text << std::endl;
    }
    file.close();
}

void Console::AddVar(ConsoleVar* newVar,
                     std::string varName)
{
    _cvars[varName] = newVar;
}

void Console::RemoveVar(std::string varName)
{
    if (_cvars.find(varName) != _cvars.end()) {
        _cvars.erase(varName);
    }
}

void Console::Show()
{
    TextManager* tMan = _locator->Get<TextManager>();
    if ( !tMan || _visible ) {
        return;
    }
    int winWidth = _locator->Get<Options>()->GetOptionDataPtr<int>("r_resolutionX");
    std::string consoleInfo = "Console:  Ingenium v.";
    consoleInfo.append(_locator->Get<Options>()->GetOptionDataPtr<std::string>("version"));
    _textWidget = new UITextInputSCB(-winWidth/2,
                                    1.0,
                                    winWidth-1,
                                    22,
                                    CONSOLE_TEXT_DEPTH,
                                    consoleInfo,
                                    &Console::OnTextInput);
    _textWidget->StartTextInput();
    _visible = true;
    Refresh();
}

void Console::Hide()
{
    TextManager* tMan = _locator->Get<TextManager>();
    if ( !tMan ) {
        return;
    }
    for (size_t i = 0; i < _textLines.size(); i++) {
        tMan->RemoveText(_textLines[i].blobID);
        _textLines[i].blobID = -1;
    }
    
    _visible = false;
    delete _textWidget;
    _textWidget = NULL;
}

void Console::Refresh()
{
    TextManager* tMan = _locator->Get<TextManager>();
    
    int msgCount = (int)_textLines.size();
    if (_visible) {
        int winWidth = _locator->Get<Options>()->GetOptionDataPtr<int>("r_resolutionX");
        double labelPosX = -winWidth / 2 + 8;    // left edge of screen
        double labelPosY = 22+(msgCount + 2)*CONSOLE_FONT_SIZE;
        // Move existing labels up
        for (int i = 0; i < msgCount; i++) {
            if (_textLines[i].blobID != -1) {  // Move text
                tMan->UpdateTextPos(_textLines[i].blobID,
                                    glm::vec3(labelPosX, labelPosY, CONSOLE_TEXT_DEPTH));
            } else {                            // Add line
                _textLines[i].blobID = tMan->AddText(_textLines[i].text,
                                                     glm::vec3(labelPosX, labelPosY, CONSOLE_TEXT_DEPTH),
                                                     true,
                                                     CONSOLE_FONT_SIZE,
                                                     FONT_PIXEL,
                                                     0.0,
                                                     _textLines[i].color);
            }
            labelPosY -= CONSOLE_FONT_SIZE;
        }
    }
}


void Console::OnTextInput(std::string input)
{
    Process(input);
    _textWidget->Clear();
    _textWidget->StopTextInput();
    _textWidget->StartTextInput();
}

void Console::Tokenize(const std::string& input,
                       std::vector<std::string>& tokens)
{
    std::string::size_type last_pos = 0;
    std::string::size_type pos = 0;
    while(true) {
        pos = input.find_first_of(' ', last_pos);
        if( pos == std::string::npos ) {
            tokens.push_back(input.substr(last_pos));
            break;
        } else {
            tokens.push_back(input.substr(last_pos, pos - last_pos));
            last_pos = pos + 1;
        }
    }
}

