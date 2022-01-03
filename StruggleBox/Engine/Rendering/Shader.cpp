#include "Shader.h"

#include "Log.h"
#include "glm/gtc/type_ptr.hpp"

Shader::Shader()
	: m_program(0)
	, m_vertexShader(0)
	, m_geometryShader(0)
	, m_fragmentShader(0)
{
}

Shader::~Shader()
{
}

void Shader::initialize(const std::string& vshSource, const std::string& fshSource)
{
	const GLchar* vs = vshSource.c_str();
	const GLchar* fs = fshSource.c_str();

	m_program = glCreateProgram();
	m_vertexShader = attach(m_program, GL_VERTEX_SHADER, &vs);
	m_fragmentShader = attach(m_program, GL_FRAGMENT_SHADER, &fs);

    linkProgram();
}

void Shader::initialize(const std::string& gshSource, const std::string& vshSource, const std::string& fshSource)
{
	const GLchar* gs = gshSource.c_str();
	const GLchar* vs = vshSource.c_str();
	const GLchar* fs = fshSource.c_str();

	m_program = glCreateProgram();
	m_geometryShader = attach(m_program, GL_GEOMETRY_SHADER, &gs);
	m_vertexShader = attach(m_program, GL_VERTEX_SHADER, &vs);
	m_fragmentShader = attach(m_program, GL_FRAGMENT_SHADER, &fs);

	linkProgram();
}

void Shader::terminate()
{
	if (m_fragmentShader)
		glDeleteShader(m_fragmentShader);
	if (m_geometryShader)
		glDeleteShader(m_geometryShader);
	if (m_vertexShader)
		glDeleteShader(m_vertexShader);
	if (m_program)
		glDeleteProgram(m_program);
}

void Shader::linkProgram()
{
	glLinkProgram(m_program);

	GLint result;
	glGetProgramiv(m_program, GL_LINK_STATUS, &result);
	if(result == GL_FALSE)
	{
		GLint length;
		char *log;
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetProgramInfoLog(m_program, length, &result, log);
		Log::Error("[Shader] Program linking failed: %s\n", log);
		free(log);
		m_program = 0;
	}
}

GLuint Shader::compile(GLenum type, const GLchar** source)
{    
	GLuint shader = glCreateShader(type);
	GLint length = (GLint)strlen((char*)*source);
	glShaderSource(shader, 1, source, &length);
	glCompileShader(shader);
    
	GLint result = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if(result == GL_TRUE)
	{
		return shader;
	}
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	log = (char*)malloc(length);
	glGetShaderInfoLog(shader, length, &result, log);
	Log::Error("[Shader] Unable to compile: %s\n", log);
	free(log);
	glDeleteShader(shader);
	return 0;
}

GLuint Shader::attach(GLuint program, GLenum type, const GLchar** source)
{
	GLuint shader = compile(type, source);
	if(shader != 0 && shader != GL_INVALID_ENUM)
	{
		glAttachShader(program, shader);
	}
    return shader;
}

GLint Shader::getAttribute(const std::string& name) const
{
	return glGetAttribLocation(m_program, name.c_str());
}

GLint Shader::getUniform(const std::string& name) const
{
	return glGetUniformLocation(m_program, name.c_str());
}

void Shader::setUniform2fv(const char* name, float x, float y) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        GLfloat vec[2] = {x, y};
        glUniform2fv( uniform, 1, vec);
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform2fv(const char* name, const glm::vec2& v) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        glUniform2fv(uniform, 1, (GLfloat*)&v);
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform3fv(const char* name, float x, float y, float z) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        GLfloat vec[3] = {x,y,z};
        glUniform3fv(uniform, 1, vec);
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform3fv(const char* name, const glm::vec3& v) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        glUniform3fv(uniform, 1, (GLfloat*)&v);
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform4fv(const char *name, float x,float y, float z, float w) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        GLfloat vec[4] = {x,y,z,w};
        glUniform4fv(uniform, 1, vec);
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform4fv(const char* name, const glm::vec4& v) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        glUniform4fv(uniform, 1, (GLfloat*)&v);
		return;
    }
    Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform4fv(const char* name, const Color& c) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        glUniform4fv(uniform, 1, &c.r);
		return;
    }
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniformM4fv(const char* name, const glm::mat4& m) const
{
    GLint uniform = getUniform(name);
    if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        glUniformMatrix4fv( uniform, 1, GL_FALSE, glm::value_ptr(m));
		return;
    }
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniformM3fv(const char* name, const glm::mat3& m) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        glUniformMatrix3fv( uniform, 1, GL_FALSE, glm::value_ptr(m));
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform1fv(const char *name, float val) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
        glUniform1fv( uniform, 1, &val);
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform1iv(const char *name, int val) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
		glUniform1iv( uniform, 1, &val);
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}

void Shader::setUniform1bv(const char *name, bool val) const
{
    GLint uniform = getUniform(name);
	if (uniform != GL_INVALID_VALUE && uniform != GL_INVALID_OPERATION)
	{
		glUniform1iv( uniform, 1, (GLint*)&val);
		return;
	}
	Log::Error("[Shader] no such uniform: %s", name);
}
