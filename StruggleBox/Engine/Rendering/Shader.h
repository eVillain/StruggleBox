#ifndef SHADER_H
#define SHADER_H

#include "GFXDefines.h"
#include "Color.h"
#include <map>
#include <vector>
#include <string>

class Shader
{
public:
    Shader();
    ~Shader();

    void initialize(
		const std::string vshSource,
		const std::string fshSource);

	void initialize(
		const std::string gshSource,
		const std::string vshSource,
		const std::string fshSource);

    inline void begin() const { glUseProgram(prog); };
    inline void end() const { glUseProgram(0); };

    inline GLuint GetVertexShader() { return vertex_shader; };
    inline GLuint GetFragmentShader() { return fragment_shader; };
    inline GLuint GetProgram() { return prog; };
    
	GLint getAttribute(const std::string name) const;
	GLint getUniform(const std::string name) const;

    void setUniform2fv(const char *name, float x,float y) const;
    void setUniform2fv(const char *name, const glm::vec2 & v) const;
    void setUniform3fv(const char *name,float x,float y, float z) const;
    void setUniform3fv(const char *name, const glm::vec3 & v) const;
    void setUniform4fv(const char *name,float x,float y, float z, float w) const;
    void setUniform4fv(const char *name, const glm::vec4 & v) const;
    void setUniform4fv(const char *name, const Color & c) const;
    void setUniformM3fv(const char *name, const glm::mat3 & m) const;
    void setUniformM4fv(const char *name, const glm::mat4 & m) const;
    void setUniform1fv(const char *name, float val ) const;
    void setUniform1iv(const char *name, int val ) const;
    void setUniform1bv(const char *name, bool val ) const;

private:
    GLuint geometry_shader;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint prog;

    GLuint compile(GLenum type, const GLchar **source);
    GLuint attach(GLuint program, GLenum type, const GLchar **source);
	void linkProgram();
};

#endif /* SHADER_H*/
