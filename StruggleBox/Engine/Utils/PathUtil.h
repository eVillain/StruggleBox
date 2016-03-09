//
//  PathUtil.h
//  DungeonSmith
//
//  Created by eVillain on 18/10/15.
//  Copyright © 2015 The Drudgerist. All rights reserved.
//

#ifndef PathUtil_h
#define PathUtil_h

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

#endif /* PathUtil_h */
