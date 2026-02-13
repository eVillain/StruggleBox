#pragma once

#include <string>
#include <map>

class Allocator;
class Shader;

class ShaderLoader
{
public:
	static Shader* load(const std::string& vshPath, const std::string& fshPath, Allocator& allocator);
	static Shader* load(const std::string& gshPath, const std::string& vshPath, const std::string& fshPath, Allocator& allocator);
	static Shader* loadFromSource(const std::string& vshSource, const std::string& fshSource, Allocator& allocator);
	static Shader* loadFromSource(const std::string& gshSource, const std::string& vshSource, const std::string& fshSource, Allocator& allocator);

private:
	static std::string loadFile(const std::string& filePath, Allocator& allocator);
};
