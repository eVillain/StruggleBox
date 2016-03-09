#include "ShaderManager.h"
#include "FileUtil.h"

/**********************************************************************
 * Default shader programs
 *********************************************************************/

const GLchar *default_vertex_shader[] = {
    "#version 330 core\n"
    "layout(location = 0)in vec4 vCoord;\n"
    "layout(location = 1) in vec4 vColor;\n"
    "out vec4 fragmentColor;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{ gl_Position = MVP * vCoord;\n"
    " fragmentColor = vColor; }"
};
const GLchar *default_frag_shader[] = {
    "#version 330 core\n"
    "in vec4 fragmentColor;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{ color = fragmentColor; }"
};

namespace ShaderManager {
    typedef std::vector<Shader*> ShaderList;

    ShaderList shaders;

    void ClearShaders() {
        if ( shaders.size() ) {
            printf("[ShaderMan] Releasing shaders...\n");
            for( unsigned int i=0; i< shaders.size(); i++ ) {
                delete shaders[i];
            }
            shaders.clear();
        }
    }
    Shader* LoadFromFile(const std::string vshPath, const std::string fshPath) {
        
        Shader* shader = new Shader();
        std::string vertShader = FileUtil::GetPath().append("Shaders/");
        vertShader.append(vshPath);
        std::string fragShader = FileUtil::GetPath().append("Shaders/");
        fragShader.append(fshPath);
//        printf("Loading shader:\n %s\n %s", vertShader.c_str(), fragShader.c_str());
        shader->InitFromFile( vertShader, fragShader );
        if (shader->GetProgram() == 0) {
            printf("Shader program loading failed, loading default\n");
            shader->InitFromSource( default_vertex_shader, default_frag_shader );
        } 
        shaders.push_back(shader);
        return shader;
    }
    Shader* LoadFromFile(const std::string gshPath, const std::string vshPath, const std::string fshPath) {
        Shader* shader = new Shader();
        std::string geomShader = FileUtil::GetPath().append("Shaders/");
        geomShader.append(gshPath);
        std::string vertShader = FileUtil::GetPath().append("Shaders/");
        vertShader.append(vshPath);
        std::string fragShader = FileUtil::GetPath().append("Shaders/");
        fragShader.append(fshPath);
        
        shader->InitFromFile( geomShader, vertShader, fragShader );
        if (shader->GetProgram() == 0) {
            printf("Shader program loading failed, loading default\n");
            shader->InitFromSource( default_vertex_shader, default_frag_shader );
        } 
        shaders.push_back(shader);
        return shader;
    }
    
    void ClearShader( Shader* oldShader ) {
        ShaderList::iterator it = std::find(shaders.begin(), shaders.end(), oldShader);
        if ( it != shaders.end() ) {
            delete *it;
            shaders.erase(it);
        }
    }
}

