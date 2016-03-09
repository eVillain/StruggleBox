#ifndef PATH_UTIL_H
#define PATH_UTIL_H

#include "FileUtil.h"

class PathUtil
{
public:
    static std::string GetShadersPath()
    {
        return FileUtil::GetPath() + "Shaders/";
    }
    
    static std::string GetFontsPath()
    {
        return FileUtil::GetPath() + "Data/Fonts/";
    }
};

#endif /* PATH_UTIL_H */
