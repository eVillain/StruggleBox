#include "ShaderLoader.h"

#include "Shader.h"
#include "FileUtil.h"
#include "Log.h"
#include "Allocator.h"
#include "ArenaOperators.h"

Shader* ShaderLoader::load(const std::string& vshFile, const std::string& fshFile, Allocator& allocator)
{
	const std::string vshPath = FileUtil::GetPath() + "Shaders/" + vshFile;
	const std::string fshPath = FileUtil::GetPath() + "Shaders/" + fshFile;

	Log::Info("[ShaderLoader] Loading shader:\n %s\n %s", vshPath.c_str(), fshPath.c_str());

	const std::string vertShader = loadFile(vshPath, allocator);
	const std::string fragShader = loadFile(fshPath, allocator);
	if (vertShader.empty() || fragShader.empty())
	{
		Log::Error("[ShaderLoader] Shader program loading failed!");
		return nullptr;
	}

	Shader* shader = CUSTOM_NEW(Shader, allocator)();
	shader->initialize(vertShader.c_str(), fragShader.c_str());

	if (shader->GetProgram() == 0)
	{
		Log::Error("[ShaderLoader] Shader program compiling failed!");
		CUSTOM_DELETE(shader, allocator);
		return nullptr;
	}

	return shader;
}

Shader* ShaderLoader::load(const std::string& gshFile, const std::string& vshFile, const std::string& fshFile, Allocator& allocator)
{
	const std::string gshPath = FileUtil::GetPath() + "Shaders/" + gshFile;
	const std::string vshPath = FileUtil::GetPath() + "Shaders/" + vshFile;
	const std::string fshPath = FileUtil::GetPath() + "Shaders/" + fshFile;

	Log::Debug("[ShaderLoader] Loading shader:\n %s\n %s\n %s", gshPath.c_str(), vshPath.c_str(), fshPath.c_str());

	const std::string geomShader = loadFile(gshPath, allocator);
	const std::string vertShader = loadFile(vshPath, allocator);
	const std::string fragShader = loadFile(fshPath, allocator);
	if (geomShader.empty() || vertShader.empty() || fragShader.empty())
	{
		Log::Error("[ShaderLoader] Shader program loading failed, loading default!");
		return nullptr;
	}
	Shader* shader = CUSTOM_NEW(Shader, allocator)();
	shader->initialize(geomShader, vertShader, fragShader);

	if (shader->GetProgram() == 0)
	{
		Log::Error("[ShaderLoader] Shader program compiling failed, loading default!");
		CUSTOM_DELETE(shader, allocator);
		return nullptr;
	}

	return shader;
}

std::string ShaderLoader::loadFile(const std::string& filePath, Allocator& allocator)
{
	if (!FileUtil::DoesFileExist(filePath, ""))
	{
		Log::Error("[ShaderLoader] File not found: %s", filePath.c_str());
		return "";
	}
	const size_t fileSize = FileUtil::GetFileSize(filePath);
	const size_t blockSize = 1024;
	char* source = (char*)allocator.allocate(fileSize + 1);
	if (!source)
	{
		Log::Error("[ShaderLoader] Unable to allocate %l bytes for reading", fileSize);
		return "";
	}

	FILE *fp;
#ifdef _WIN32
	fopen_s(&fp, filePath.c_str(), "r");
#else
	fp = fopen(filePath.c_str(), "r");
#endif
	if (!fp)
	{
		Log::Error("[ShaderLoader] Unable to open %s for reading", filePath.c_str());
		return "";
	}

	size_t blockBytes, readBytes = 0;
	while ((blockBytes = fread(source+readBytes, 1, blockSize, fp)) > 0)
	{
		readBytes += blockBytes;
	}

	fclose(fp);
	source[readBytes] = '\0';
	std::string data(source);
	allocator.deallocate(source);

	return data;
}
