#include "Shader.h"
#include "Log.h"
#include "glm/gtc/type_ptr.hpp"

Shader::Shader()
{
    geometry_shader = 0;
    vertex_shader = 0;
    fragment_shader = 0;
    prog = 0;
}

Shader::~Shader()
{
    if (geometry_shader) glDeleteShader(geometry_shader);
    geometry_shader = 0;
    if (vertex_shader) glDeleteShader(vertex_shader);
    vertex_shader = 0;
    if (fragment_shader) glDeleteShader(fragment_shader);
    fragment_shader = 0;

	if (prog) glDeleteProgram(prog);
    prog = 0;
}

void Shader::initialize(
	const std::string vshSource,
	const std::string fshSource)
{
	const GLchar* vs = vshSource.c_str();
	const GLchar* fs = fshSource.c_str();

	prog = glCreateProgram();

	vertex_shader = attach( prog, GL_VERTEX_SHADER, &vs );
	fragment_shader = attach( prog, GL_FRAGMENT_SHADER, &fs );

    linkProgram();
}

void Shader::initialize(
	const std::string gshSource,
	const std::string vshSource,
	const std::string fshSource)
{
	const GLchar* gs = gshSource.c_str();
	const GLchar* vs = vshSource.c_str();
	const GLchar* fs = fshSource.c_str();

	prog = glCreateProgram();

	//Log::Debug("geom: %s", gs);
	geometry_shader = attach(prog, GL_GEOMETRY_SHADER, &gs);
	//Log::Debug("vert: %s", vs);
	vertex_shader = attach(prog, GL_VERTEX_SHADER, &vs);
	//Log::Debug("frag: %s", fs);
	fragment_shader = attach(prog, GL_FRAGMENT_SHADER, &fs);

	linkProgram();
}

void Shader::linkProgram()
{
	glLinkProgram(prog);

	GLint result;
	glGetProgramiv(prog, GL_LINK_STATUS, &result);
	if(result == GL_FALSE)
	{
		GLint length;
		char *log;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetProgramInfoLog(prog, length, &result, log);
		Log::Error("[Shader] Program linking failed: %s\n", log);
		free(log);
        prog = 0;
	}
}

GLuint Shader::compile(
	GLenum type,
	const GLchar **source)
{
	GLuint shader;
    GLint length;
    GLint result;
    
	/* create shader object, set the source, and compile */
	shader = glCreateShader(type);
	length = (GLint)strlen((char*)*source);
	glShaderSource(shader, 1, source, &length);
	glCompileShader(shader);
    
	/* make sure the compilation was successful */
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE) {
		char *log;
        
		/* get the shader info log */
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetShaderInfoLog(shader, length, &result, log);
        
		/* print an error message and the info log */
		Log::Error("[Shader] Unable to compile: %s\n", log);
        
		free(log);
        
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint Shader::attach(
	GLuint program,
	GLenum type,
	const GLchar **source)
{
	/* compile the shader */
	GLuint shader = compile(type, source);
    
	if(shader != 0) {
		/* attach the shader to the program */
		glAttachShader(program, shader);
	}
    return shader;
}

GLint Shader::getAttribute(const std::string name) const
{
	return glGetAttribLocation(prog, name.c_str());
}

GLint Shader::getUniform(const std::string name) const
{
	return glGetUniformLocation(prog, name.c_str());
}

void Shader::setUniform2fv(
	const char *name,
	float x,
	float y) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[2] = {x, y};
        glUniform2fv( uniform, 1, vec);
    }
}

void Shader::setUniform2fv(
	const char *name,
	const glm::vec2 & v) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[2] = {v.x, v.y};
        glUniform2fv(uniform, 1, vec);
    }
}

void Shader::setUniform3fv(
	const char *name,
	float x,
	float y,
	float z) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[3] = {x,y,z};
        glUniform3fv( uniform, 1, vec);
    }
}

void Shader::setUniform3fv(
	const char *name,
	const glm::vec3 & v) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[3] = {v.x,v.y,v.z};
        glUniform3fv( uniform, 1, vec);
    }
}

void Shader::setUniform4fv(
	const char *name,
	float x,
	float y,
	float z,
	float w) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[4] = {x,y,z,w};
        glUniform4fv( uniform, 1, vec);
    }
}

void Shader::setUniform4fv(
	const char *name,
	const glm::vec4 & v) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[4] = {v.x,v.y,v.z,v.w};
        glUniform4fv( uniform, 1, vec);
    } else {
        printf("no such uniform: %s\n", name);
    }
}

void Shader::setUniform4fv(
	const char *name,
	const Color & c) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        glUniform4fv( uniform, 1, &c.r);
    } else {
        printf("no such uniform: %s\n", name);
    }
}

void Shader::setUniformM4fv(
	const char *name,
	const glm::mat4 & m) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        glUniformMatrix4fv( uniform, 1, GL_FALSE, glm::value_ptr( m ));
    } else {
        printf("no such uniform: %s\n", name);
    }
}

void Shader::setUniformM3fv(
	const char *name,
	const glm::mat3 & m) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        glUniformMatrix3fv( uniform, 1, GL_FALSE, glm::value_ptr( m ));
    } else {
        printf("no such uniform: %s\n", name);
    }
}

void Shader::setUniform1fv(
	const char *name,
	float val ) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        glUniform1fv( uniform, 1, &val);
    }
}

void Shader::setUniform1iv(
	const char *name,
	int val ) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        glUniform1iv( uniform, 1, &val);
    }
}

void Shader::setUniform1bv(
	const char *name,
	bool val ) const
{
    GLuint uniform = getUniform(name);
    if ( uniform != -1 ) {
        glUniform1iv( uniform, 1, (GLint*)&val);
    }
}
