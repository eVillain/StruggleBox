//
//  ShaderManager.h
//  Bloxelizer
//
//  Created by Ville-Veikko Urrila on 9/7/12.
//  Copyright (c) 2012 The Drudgerist. All rights reserved.
//

#ifndef Bloxelizer_ShaderManager_h
#define Bloxelizer_ShaderManager_h
#include <map>
#include <vector>
#include <string>
#include "Shader.h"

namespace ShaderManager {
    /*
     * Clears all the shaders and deletes
     * them from the list.
     */
    void ClearShaders();
    
    Shader* LoadFromFile(const std::string vshPath, const std::string fshPath);
    Shader* LoadFromFile(const std::string gshPath, const std::string vshPath, const std::string fshPath);    
    void ClearShader( Shader* oldShader );
    
};

#endif
