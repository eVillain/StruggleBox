//
//  Shader.h
//  Bloxelizer
//
//  Created by Ville-Veikko Urrila on 9/7/12.
//  Copyright (c) 2012 The Drudgerist. All rights reserved.
//

#ifndef Bloxelizer_Shader_h
#define Bloxelizer_Shader_h
#include <map>
#include <vector>
#include <string>
#include "GFXDefines.h"

class Shader {
private:
    GLuint geometry_shader;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint prog;
    /*
     * Returns a string containing the text in
     * a vertex/fragment shader source file.
     */
    GLchar* ShaderLoadSource( const std::string filePath );
    /*
     * Returns a shader object containing a shader
     * compiled from the given GLSL shader file.
     */
    GLuint ShaderCompileFromSource(GLenum type, const GLchar **source);
    //    GLuint ShaderCompileFromFile(GLenum type, const char *filePath);
    /*
     * Compiles and attaches a shader of the
     * given type to the given program object.
     */
    GLuint ShaderAttachFromSource(GLuint program, GLenum type, const GLchar **source);
    GLuint ShaderAttachFromFile(GLuint program, GLenum type, const std::string filePath);
public:
    // constructor
    Shader();
    // destructor
    ~Shader();
    
    void InitFromFile( const std::string vshPath, const std::string fshPath );
    void InitFromFile( const std::string gshPath, const std::string vshPath, const std::string fshPath );
    void InitFromSource( const GLchar** vshSource, const GLchar** fshSource );
    void LinkProgram();

    inline void Begin() const { glUseProgram(prog); };
    inline void End() const { glUseProgram(0); };
    inline GLuint GetVertexShader() { return vertex_shader; };
    inline GLuint GetFragmentShader() { return fragment_shader; };
    inline GLuint GetProgram() { return prog; };
    
    GLint GetAttribute( const std::string name ) const {
        return glGetAttribLocation( prog, name.c_str() );
    };
    GLint GetUniform( const std::string name ) const {
        return glGetUniformLocation( prog, name.c_str() );
    };
    void setUniform2fv(const char *name, float x,float y) const;
    void setUniform2fv(const char *name, const glm::vec2 & v) const;
    void setUniform3fv(const char *name,float x,float y, float z) const;
    void setUniform3fv(const char *name, const glm::vec3 & v) const;
    void setUniform4fv(const char *name,float x,float y, float z, float w) const;
    void setUniform4fv(const char *name, const glm::vec4 & v) const;
    void setUniform4fv(const char *name, const Color & c) const;
    void setUniformM3fv(const char *name, const glm::mat3 & m) const;
    void setUniformM4fv(const char *name, const glm::mat4 & m) const;
//    void setUniform(const char *name, const mat3 & m);
    void setUniform1fv(const char *name, float val ) const;
    void setUniform1iv(const char *name, int val ) const;
    void setUniform1bv(const char *name, bool val ) const;
};

#endif
