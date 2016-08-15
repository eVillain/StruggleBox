#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <string>
#include <vector>
#include <fstream>

class FileUtil
{
public:
    // Utility functions for file system
    static bool GetAllFiles(
		const std::string dir,
		std::vector<std::string> &fileNames);

    static bool GetFilesOfType(
		const std::string dir,
		const std::string type,
		std::vector<std::string> &fileNames);

	static bool GetSubfolders(
		const std::string dir,
		std::vector<std::string> &fileNames);

    static bool DoesFolderExist(const std::string dir);

    static bool DoesFileExist(
		const std::string dir,
		const std::string fileName);

    static bool CreateFolder(const std::string dir);

	static long GetFileSize(const std::string filename);

	static std::string GetContainingFolder(const std::string path);

    // Application file system path
	static std::string GetPath();
private:
	static void UpdateRelativePath();

    static std::string relativePath;
};

#endif /* FILE_UTIL_H */
