#include "CommandProcessor.h"
#include "StringUtil.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h> // needed for getopt
#endif


// The command map (string holds the name of the command)
CommandProcessor::CommandMap CommandProcessor::commandMap;
// The command buffer, contains next command to execute at front
std::deque<std::string> CommandProcessor::commandBuffer;

void CommandProcessor::Initialize()
{
    Log::Debug("[CommandProcessor] initializing...");
    AddCommand("commands", Command<>([&]() {
        CommandMap::const_iterator it;
        //Console::PrintString("Commands:", COLOR_WHITE);
        //for ( it = commandMap.begin(); it != commandMap.end(); it++ ) {
        //    Console::PrintString( it->first, COLOR_WHITE );
        //}
    }));
    AddCommand("print", Command<const std::string>([&](const std::string& text) {
        //Console::PrintString( text, COLOR_GREEN );
    }));
}

void CommandProcessor::Terminate()
{
    Log::Debug("[CommandProcessor] terminating...");
    commandBuffer.clear();
}


void CommandProcessor::Update(double delta)
{
    if ( !commandBuffer.empty() ) {
        std::string nextCmd = commandBuffer.front();
        commandBuffer.pop_front();

        Process(nextCmd);
    }
}

//
// COMMAND OPERATIONS
//

void CommandProcessor::RemoveCommand(const std::string& cmdName) {
    CommandMap::const_iterator it = commandMap.find(cmdName);
    if(it != commandMap.end())
    {
        commandMap.erase(it);
    }
}

//
// BUFFER OPERATIONS
//
void CommandProcessor::Process( const std::string& command )
{
    Log::Debug("Command: %s", command.c_str());
    // Try to tokenize input text
    std::deque<std::string> tokens;
    StringUtil::Tokenize(command, tokens);
    if ( tokens.size() == 0 ) return;

    // Assume first token is the command name, check others for data
    if ( tokens.size() == 1 ) {
        ExecuteCommand(tokens[0]); // Single word command, try to execute
    } else {
        // TODO: We should run a for loop over the tokens and convert them into
        // our datatypes as needed to make this cleaner and calling different
        // argument combinations easier
        
        if ( tokens.size() == 2 ) {
            if ( StringUtil::IsNumber(tokens[1]) ) {
                if ( tokens[1].find('.') != std::string::npos ) {
                    auto tokenData = std::stof(tokens[1]);
                    ExecuteCommand(tokens[0], tokenData);
                } else if (StringUtil::IsNumber(tokens[1])) {
                    auto tokenData = std::stoi(tokens[1]);
                    ExecuteCommand(tokens[0], tokenData);
                } else {
                    ExecuteCommand(tokens[0], tokens[1]);
                }
            } else {
                ExecuteCommand(tokens[0], tokens[1]);
            }
            
        } else if ( tokens.size() == 3 ) {
            if ( tokens[1].find('.') != std::string::npos ) {
                float tokenData1 = std::stof(tokens[1]);
                if ( tokens[2].find('.') != std::string::npos ) {
                    float tokenData2 = std::stof(tokens[2]);
                    ExecuteCommand(tokens[0], tokenData1, tokenData2);
                } else {
                    int tokenData2 = std::stoi(tokens[2]);
                    ExecuteCommand(tokens[0], tokenData1, tokenData2);
                }
            } else if (StringUtil::IsNumber(tokens[1])) {
                int tokenData1 = std::stoi(tokens[1]);
                if ( tokens[2].find('.') != std::string::npos ) {
                    float tokenData2 = std::stof(tokens[2]);
                    ExecuteCommand(tokens[0], tokenData1, tokenData2);
                } else {
                    int tokenData2 = std::stoi(tokens[2]);
                    ExecuteCommand(tokens[0], tokenData1, tokenData2);
                }
            } else {    // String token
                if ( tokens[2].find('.') != std::string::npos ) {
                    float tokenData2 = std::stof(tokens[2]);
                    ExecuteCommand(tokens[0], tokens[1], tokenData2);
                } else if (StringUtil::IsNumber(tokens[2])) {
                    int tokenData2 = std::stoi(tokens[2]);
                    ExecuteCommand(tokens[0], tokens[1], tokenData2);
                } else {
                    ExecuteCommand(tokens[0], tokens[1], tokens[2]);
                }
            }
        }
    }
}

void CommandProcessor::Buffer(const std::string& input)
{
    // Add lines to end of our queue
    StringUtil::SplitIntoLines(input, commandBuffer);
}

// TODO: Process tokens here
void CommandProcessor::Buffer(const int argc, const char * argv[])
{
    // Copy all arguments into a container of strings
    std::deque<std::string> allArgs(argv, argv + argc);
    
    // Iterate through the arguments and buffer commands
    std::string nextCommand;
    // Skip the first one, it's the application path
    std::deque<std::string>::iterator it = allArgs.begin() + 1;
    for (; it != allArgs.end(); it++)
    {
        std::string token = *it;
        if ( token.length() == 0 ) continue;
        //std::cout << "Got arg: " << token << std::endl;

        // Check if this token begins a new command
        if ( token.at(0) == '-' )
        {
            // If we already had a command, buffer it and clean up
            if ( nextCommand.length() )
            {
                Buffer(nextCommand);
                nextCommand.clear();
            }
            nextCommand += token.erase(0,1);
        }
        else
        {
            if ( nextCommand.length() == 0 )
            {
                std::cout << "I don't know what you want: " << token << std::endl;
            }
            else
            {
                nextCommand += " " + token;
            }
        }
    }
    // If we already had a command, buffer it and clean up
    if ( nextCommand.length() )
    {
        Buffer(nextCommand);
        nextCommand.clear();
    }
}

