#ifndef PATH_UTIL_H
#define PATH_UTIL_H

#include "FileUtil.h"

class PathUtil
{
public:
	static std::string GUIPath()
	{
		return FileUtil::GetPath() + "Data/GUI/";
	}

	static std::string ParticlesPath()
	{
		return FileUtil::GetPath() + "Data/Particles/";
	}

	static std::string MaterialsPath()
	{
		return FileUtil::GetPath() + "Data/Materials/";
	}

	static std::string ObjectsPath()
	{
		return FileUtil::GetPath() + "Data/Objects/";
	}

	static std::string getObjectPath(const std::string& fileName)
	{
		std::string absolutePath;
		if (fileName.find("\\") == std::string::npos &&
			fileName.find("/") == std::string::npos)
		{
			absolutePath = FileUtil::GetPath() + "Data/Objects/" + fileName;
		}
		else {
			absolutePath = fileName;
		}
		return absolutePath;
	}

    static std::string ShadersPath()
    {
        return FileUtil::GetPath() + "Shaders/";
    }
    
    static std::string FontsPath()
    {
        return FileUtil::GetPath() + "Data/Fonts/";
    }
};

#endif /* PATH_UTIL_H */
