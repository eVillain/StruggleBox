#include "ShaderCache.h"

#include "Log.h"

const ShaderID ShaderCache::NO_SHADER_ID = 0;

ShaderCache::ShaderCache()
	: m_nextShaderID(1)
{
}
ShaderID ShaderCache::addShader(Shader* shader, const std::string& vertexName, const std::string& fragmentName)
{
	const std::string combinedName = vertexName + fragmentName;
	const std::size_t nameHash = std::hash<std::string>{}(combinedName);
	return addShader(shader, nameHash);
}
ShaderID ShaderCache::addShader(Shader* shader, const std::string& geometryName, const std::string& vertexName, const std::string& fragmentName)
{
	const std::string combinedName = geometryName + vertexName + fragmentName;
	const std::size_t nameHash = std::hash<std::string>{}(combinedName);
	return addShader(shader, nameHash);
}

ShaderID ShaderCache::getShaderID(const std::string& vertexName, const std::string& fragmentName)
{
	const std::string combinedName = vertexName + fragmentName;
	const std::size_t nameHash = std::hash<std::string>{}(combinedName);
	return getShaderID(nameHash);
}

ShaderID ShaderCache::getShaderID(const std::string& geometryName, const std::string& vertexName, const std::string& fragmentName)
{
	const std::string combinedName = geometryName + vertexName + fragmentName;
	const std::size_t nameHash = std::hash<std::string>{}(combinedName);
	return getShaderID(nameHash);
}

Shader* ShaderCache::getShaderByID(const ShaderID ShaderID)
{
	const auto it = m_shaders.find(ShaderID);
	if (it != m_shaders.end())
	{
		return it->second;
	}
	return nullptr;
}

ShaderID ShaderCache::addShader(Shader* shader, const size_t nameHash)
{
	const ShaderID prevID = getShaderID(nameHash);
	if (prevID != NO_SHADER_ID)
	{
		Log::Warn("Shader with name %i already cached!", nameHash);
		return NO_SHADER_ID;
	}
	const ShaderID shaderID = m_nextShaderID;
	m_nextShaderID++;
	m_shaders[shaderID] = shader;
	m_shaderNames[nameHash] = shaderID;
	return shaderID;
}

ShaderID ShaderCache::getShaderID(const size_t nameHash)
{
	const auto it = m_shaderNames.find(nameHash);
	if (it != m_shaderNames.end())
	{
		return it->second;
	}
	return NO_SHADER_ID;
}
