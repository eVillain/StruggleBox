#pragma once

#include "RendererDefines.h"
#include <map>
#include <string>

class Shader;

class ShaderCache
{
public:
	static const ShaderID NO_SHADER_ID;

	ShaderCache();

	ShaderID addShader(Shader* shader, const std::string& vertexName, const std::string& fragmentName);
	ShaderID addShader(Shader* shader, const std::string& geometryName, const std::string& vertexName, const std::string& fragmentName);

	ShaderID getShaderID(const std::string& vertexName, const std::string& fragmentName);
	ShaderID getShaderID(const std::string& geometryName, const std::string& vertexName, const std::string& fragmentName);
	Shader* getShaderByID(const ShaderID shaderID);

private:
	ShaderID m_nextShaderID;

	std::map<ShaderID, Shader*> m_shaders;
	std::map<size_t, ShaderID> m_shaderNames;

	ShaderID addShader(Shader* shader, const size_t nameHash);
	ShaderID getShaderID(const size_t nameHash);
};

