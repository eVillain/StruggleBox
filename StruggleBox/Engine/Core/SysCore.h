#ifndef NGN_SYSCORE_H
#define NGN_SYSCORE_H

// TODO: Replace all usages of this class with FileUtil, Timer and Random

#include <string>
#include <vector>
#include <stdint.h>

namespace SysCore {
    // Application file system path
	std::string GetPath();
    void SetRelativePath();
    // Retrieve files in directory
    bool GetAllFiles(const std::string dir, std::vector<std::string> &fileNames);
    bool GetFilesOfType(const std::string dir, const std::string type, std::vector<std::string> &fileNames);
    bool DoesFolderExist( const std::string dir );
    bool DoesFileExist( const std::string dir, const std::string fileName );
    bool CreateFolder( const std::string dir );
    // Universal timers
    long GetMicroseconds();
    double GetMilliseconds();
    double GetSeconds();
    std::string GetTimeStamp();
    // Random number generators
    uint32_t RandomBits();
    double RandomDouble();
    int RandomInt(int min, int max);
    void RandomSeed (int seed);
};
#endif