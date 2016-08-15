#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <memory>
#include <vector>
#include <string>

class Shader;

class ShaderManager
{
public:
	ShaderManager() {};
	~ShaderManager() {};

	std::shared_ptr<Shader> load(
		const std::string vshPath,
		const std::string fshPath);

	std::shared_ptr<Shader> load(
		const std::string gshPath,
		const std::string vshPath,
		const std::string fshPath);

private:
	std::string loadFile(const std::string filePath);
};

#endif
