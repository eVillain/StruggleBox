#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "Log.h"
#include <iostream>
#include <string>
#include <map>
#include <deque>
#include <future>


///  Holds a list of commands tied to functions within the engine, as well as a
///  buffer of commands to execute.
///  Update(double delta) should be called within regular intervals
///
///  Note: The usage of AddCommand takes the TYPE Command / InstanceCommand as
///  an argument - ie. do not call it with 'new Command<>()' just 'Command<>()'
///
///  Usage for static functions (this function takes an std::string as an argument):
/// CommandProcessor::AddCommand("commandName1", Command<std::string>(StaticFunction));
// CommandProcessor::Execute("commandName1", std::string("This string gets passed to the function"));
///
///  Usage for member functions (where std::string is the input for those functions):
/// CommandProcessor::AddCommand("commandName2", Command<ClassName*,std::string>(&ClassName::MemberFunction));
/// CommandProcessor::Execute("commandName2", instancePtr, std::string("This string gets passed to the function"));
///  Note: THIS THING IS USELESS, WE WONT HAVE THE INSTANCE POINTER IN OUR COMMAND PROCESSOR
///
///  Usage for lambda functions / calling a particular instance member function
/// CommandProcessor::AddCommand("commandName3", Command<>([&](){ this->MemberFunction(); }));
/// CommandProcessor::Execute("commandName3");
///
///  Usage for member functions in a specific instance with parameters
/// CommandProcessor::AddCommand("commandName2", InstanceCommand<ClassName*,std::string>(&ClassName::MemberFunction, instancePtr));
/// CommandProcessor::Execute("commandName2", std::string("This string gets passed to the function"));


// Base command class for common inheritance
class CommandBase
{
public:
    virtual ~CommandBase() {}
};

// Command class, takes a variable number of arguments
template <typename... ArgTypes>
class Command : public CommandBase
{
private:
    typedef std::function<void(ArgTypes...)> FunctionType;
    FunctionType functionPtr;
public:
    Command() {};
    Command(FunctionType f) : functionPtr(f) {};
    void operator()(ArgTypes... args) { if(functionPtr) functionPtr(args...); };
};

// InstanceCommand class, takes a variable number of arguments
// and an instance pointer on which to call the method
template <typename ClassType, typename... ArgTypes>
class InstanceCommand : public CommandBase
{
private:
    typedef std::function<void(ArgTypes...)> FunctionType;
    ClassType* instancePtr;
    FunctionType functionPtr;
public:
    InstanceCommand() {};
    InstanceCommand(FunctionType f, ClassType* instance) : instancePtr(instance), functionPtr(f) {};
    void operator()(ArgTypes... args) { if(functionPtr) instancePtr->functionPtr(args...); };
};

// Command processor class
class CommandProcessor
{
public:
    static void Initialize();
    static void Terminate();
    
    // Call this update with regular intervals
    // It will process and execute the next command in the buffer
    static void Update(double delta);
    
    // Adds a new command to the map
    template <class T>
    static void AddCommand(const std::string& cmdName, const T& cmd)
    {
        commandMap.insert(std::pair<std::string, CommandBasePtr>(cmdName, CommandBasePtr(new T(cmd))));
    }
    
    // Executes a command with arguments
    template <class... ArgTypes>
    static void ExecuteCommand( const std::string& cmdName, ArgTypes... args )
    {
        typedef Command<ArgTypes...> CommandType;
        CommandMap::const_iterator it = commandMap.find(cmdName);
        if ( it != commandMap.end())
        {
            if ( it->second.get() != NULL ) {
                CommandType* c = (CommandType*)(it->second.get());
                if (c) {
                    (*c)(args...);
                } else {
                    Log::Warn("[CmdProcessor] NULL command exec - %s", cmdName.c_str());
                }
            } else {
                Log::Warn("[CmdProcessor] Bad command exec - %s", cmdName.c_str());
            }
        } else {
            Log::Warn("Unknown command: %s", cmdName.c_str());
        }
    }
    // Removes a command from the map
    static void RemoveCommand(const std::string& cmdName);
    
    // Process and execute the given command
    static void Process(const std::string& command);
    // Buffer raw text into the buffer
    static void Buffer(const std::string& input);
    // Buffer command-line arguments into buffer
    static void Buffer(const int argc, const char * arg[] );
    
    static size_t GetBufferCount() { return commandBuffer.size(); };
    
    typedef std::shared_ptr<CommandBase> CommandBasePtr;
    typedef std::map<const std::string, CommandBasePtr> CommandMap;
private:
    // The command map (string holds the name of the command)
    static CommandMap commandMap;
    // The command buffer, contains next command to execute at front
    static std::deque<const std::string> commandBuffer;
};

#endif /* COMMAND_PROCESSOR_H */
