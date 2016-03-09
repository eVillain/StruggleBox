#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <string>
#include <vector>

class FileUtil
{
public:
    // Utility functions for file system
    static bool GetAllFiles(const std::string dir, std::vector<std::string> &fileNames);
    static bool GetFilesOfType(const std::string dir, const std::string type, std::vector<std::string> &fileNames);
    static bool DoesFolderExist( const std::string dir );
    static bool DoesFileExist( const std::string dir, const std::string fileName );
    static bool CreateFolder( const std::string dir );
    // Application file system path
	static std::string GetPath();
    static void UpdateRelativePath();
private:
    static std::string relativePath;
};

#endif /* FILE_UTIL_H */
