//
//  Shader.cpp
//  Bloxelizer
//
//  Created by Ville-Veikko Urrila on 9/7/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "Shader.h"
#include "glm/gtc/type_ptr.hpp"

Shader::Shader() {
    geometry_shader = 0;
    vertex_shader = 0;
    fragment_shader = 0;
    prog = 0;
}
Shader::~Shader() {
//    printf("Shader %i erased\n", prog);
    /* delete the shader - it won't actually be
     * destroyed until the program that it's attached
     * to has been destroyed */
    if (geometry_shader) glDeleteShader(geometry_shader);
    geometry_shader = 0;
    if (vertex_shader) glDeleteShader(vertex_shader);
    vertex_shader = 0;
    if (fragment_shader) glDeleteShader(fragment_shader);
    fragment_shader = 0;
    /* delete the program */
    if (prog) glDeleteProgram(prog);
    prog = 0;
}
void Shader::InitFromFile( const std::string vshPath, const std::string fshPath ) {
	/* create program object and attach shaders */
    prog = glCreateProgram();
	vertex_shader = ShaderAttachFromFile( prog, GL_VERTEX_SHADER, vshPath.c_str() );
	fragment_shader = ShaderAttachFromFile( prog, GL_FRAGMENT_SHADER, fshPath.c_str() );
    if ( vertex_shader != 0 && fragment_shader != 0 ) {
        LinkProgram();
    } else {
        glDeleteProgram(prog);
        prog = 0;
    }
}
void Shader::InitFromFile( const std::string gshPath, const std::string vshPath, const std::string fshPath ) {
	/* create program object and attach shaders */
    prog = glCreateProgram();
    geometry_shader = ShaderAttachFromFile( prog, GL_GEOMETRY_SHADER, gshPath.c_str() );
	vertex_shader = ShaderAttachFromFile( prog, GL_VERTEX_SHADER, vshPath.c_str() );
	fragment_shader = ShaderAttachFromFile( prog, GL_FRAGMENT_SHADER, fshPath.c_str() );
    if ( geometry_shader != 0 && vertex_shader != 0 && fragment_shader != 0 ) {
//        glProgramParameteriEXT(prog, GL_GEOMETRY_INPUT_TYPE_EXT, GL_LINES);
//        glProgramParameteriEXT(prog, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

        LinkProgram();
    } else {
        glDeleteProgram(prog);
        prog = 0;
    }
}
void Shader::InitFromSource( const GLchar** vshSource, const GLchar** fshSource ) {
	/* create program object and attach shaders */
    prog = glCreateProgram();
	vertex_shader = ShaderAttachFromSource( prog, GL_VERTEX_SHADER, vshSource );
	fragment_shader = ShaderAttachFromSource( prog, GL_FRAGMENT_SHADER, fshSource );
    LinkProgram();
}
void Shader::LinkProgram() {
    GLint result;
    /* link the program and make sure that there were no errors */
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &result);
	if(result == GL_FALSE) {
		GLint length;
		char *log;
		/* get the program info log */
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetProgramInfoLog(prog, length, &result, log);
		/* print an error message and the info log */
		fprintf(stderr, "LinkProgram(): Program linking failed: %s\n", log);
		free(log);
        prog = 0;
	}
}
    
GLchar* Shader::ShaderLoadSource( const std::string filePath ) {
	const size_t blockSize = 512;
	FILE *fp;
	char buf[blockSize];
	char *source = NULL;
	size_t tmp, sourceLength = 0;
    std::string fullPath = "";
    fullPath.append(filePath);
    
#ifdef _WIN32
	/* open file */
	fopen_s(&fp, fullPath.c_str(), "r");
#else
	fp = fopen(fullPath.c_str(), "r");
#endif
	if(!fp) {
		fprintf(stderr, "ShaderLoadSource(): Unable to open %s for reading\n", fullPath.c_str());
		return NULL;
	}
    
	/* read the entire file into a string */
	while((tmp = fread(buf, 1, blockSize, fp)) > 0) {
		char *newSource = (char*)malloc(sourceLength + tmp + 1);
		if(!newSource) {
			fprintf(stderr, "ShaderLoadSource(): malloc failed\n");
			if(source)
				free(source);
			return NULL;
		}
        
		if(source) {
			memcpy(newSource, source, sourceLength);
			free(source);
		}
		memcpy(newSource + sourceLength, buf, tmp);
        
		source = newSource;
		sourceLength += tmp;
	}
    
	/* close the file and null terminate the string */
	fclose(fp);
	if (source) { source[sourceLength] = '\0'; }
    
	return source;
}
GLuint Shader::ShaderCompileFromSource(GLenum type, const GLchar **source) {
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
		fprintf(stderr, "ShaderCompileFromSource(): Unable to compile: %s\n", log);
        
//        printf("Got source:\n %s", *source);
		free(log);
        
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
GLuint Shader::ShaderAttachFromSource(GLuint program, GLenum type, const GLchar **source) {
	/* compile the shader */
	GLuint shader = ShaderCompileFromSource(type, source);
    
	if(shader != 0) {
		/* attach the shader to the program */
		glAttachShader(program, shader);
	}
    return shader;
}
GLuint Shader::ShaderAttachFromFile(GLuint program, GLenum type, const std::string filePath) {
    // load the file
    /* get shader source */
    const GLchar *source = ShaderLoadSource(filePath);
	if( !source ) {
        printf("Shader: Unable to load file: %s\n", filePath.c_str() );
		return 0;
    } else {
        GLuint shader = ShaderAttachFromSource(program, type, (const GLchar**)&source);
        free((char*)source);
        if ( shader == 0 ) {
            printf("Shader: Unable to compile file: %s\n", filePath.c_str() );
        }
        return shader;
    }
}
void Shader::setUniform2fv(const char *name,float x,float y ) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[2] = {x,y};
        glUniform2fv( uniform, 1, vec);
    }
}
void Shader::setUniform2fv(const char *name, const glm::vec2 & v) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[2] = {v.x,v.y};
        glUniform2fv( uniform, 1, vec);
    }
}
void Shader::setUniform3fv(const char *name,float x,float y, float z) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[3] = {x,y,z};
        glUniform3fv( uniform, 1, vec);
    }
}
void Shader::setUniform3fv(const char *name, const glm::vec3 & v) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[3] = {v.x,v.y,v.z};
        glUniform3fv( uniform, 1, vec);
    }
}
void Shader::setUniform4fv(const char *name,float x,float y, float z, float w) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[4] = {x,y,z,w};
        glUniform4fv( uniform, 1, vec);
    }
}
void Shader::setUniform4fv(const char *name, const glm::vec4 & v) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        GLfloat vec[4] = {v.x,v.y,v.z,v.w};
        glUniform4fv( uniform, 1, vec);
    } else {
        printf("no such uniform: %s\n", name);
    }
}
void Shader::setUniform4fv(const char *name, const Color & c) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        glUniform4fv( uniform, 1, &c.r);
    } else {
        printf("no such uniform: %s\n", name);
    }
}
//    void Shader::setUniform(const char *name, const vec4 & v);
void Shader::setUniformM4fv(const char *name, const glm::mat4 & m) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        glUniformMatrix4fv( uniform, 1, GL_FALSE, glm::value_ptr( m ));
    } else {
        printf("no such uniform: %s\n", name);
    }
}
void Shader::setUniformM3fv(const char *name, const glm::mat3 & m) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        glUniformMatrix3fv( uniform, 1, GL_FALSE, glm::value_ptr( m ));
    } else {
        printf("no such uniform: %s\n", name);
    }
}
//    void Shader::setUniform(const char *name, const mat3 & m);
void Shader::setUniform1fv(const char *name, float val ) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        glUniform1fv( uniform, 1, &val);
    }
}
void Shader::setUniform1iv(const char *name, int val ) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        glUniform1iv( uniform, 1, &val);
    }
}
void Shader::setUniform1bv(const char *name, bool val ) const {
    GLuint uniform = GetUniform(name);
    if ( uniform != -1 ) {
        glUniform1iv( uniform, 1, (GLint*)&val);
    }
}
