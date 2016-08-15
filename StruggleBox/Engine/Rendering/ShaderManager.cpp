#include "ShaderManager.h"
#include "Shader.h"
#include "FileUtil.h"
#include "Log.h"

/**********************************************************************
 * Default shader programs
 *********************************************************************/

const std::string default_vertex_shader = {
    "#version 400\n"
    "layout(location = 0)in vec4 vCoord;\n"
    "layout(location = 1) in vec4 vColor;\n"
    "out vec4 fragmentColor;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{ gl_Position = MVP * vCoord;\n"
    " fragmentColor = vColor; }"
};

const std::string default_frag_shader = {
    "#version 400\n"
    "in vec4 fragmentColor;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{ color = fragmentColor; }"
};

std::shared_ptr<Shader> ShaderManager::load(
	const std::string vshFile,
	const std::string fshFile)
{

	std::shared_ptr<Shader> shader = std::make_shared<Shader>();
	std::string vshPath = FileUtil::GetPath().append("Shaders/");
	vshPath.append(vshFile);
	std::string fshPath = FileUtil::GetPath().append("Shaders/");
	fshPath.append(fshFile);

	Log::Debug("[ShaderManager] Loading shader:\n %s\n %s",
		vshPath.c_str(),
		fshPath.c_str());

	std::string vertShader = loadFile(vshPath);
	std::string fragShader = loadFile(fshPath);

	shader->initialize(
		vertShader.c_str(),
		fragShader.c_str());

	if (shader->GetProgram() == 0)
	{
		Log::Warn("[ShaderManager] Shader program loading failed, loading default!");
		shader->initialize(default_vertex_shader, default_frag_shader);
	}
	return shader;
}

std::shared_ptr<Shader> ShaderManager::load(
	const std::string gshFile,
	const std::string vshFile,
	const std::string fshFile)
{
	std::shared_ptr<Shader> shader = std::make_shared<Shader>();
	std::string gshPath = FileUtil::GetPath().append("Shaders/" + gshFile);
	std::string vshPath = FileUtil::GetPath().append("Shaders/" + vshFile);
	std::string fshPath = FileUtil::GetPath().append("Shaders/" + fshFile);

	Log::Debug("[ShaderManager] Loading shader:\n %s\n %s\n %s",
		gshPath.c_str(),
		vshPath.c_str(),
		fshPath.c_str());

	std::string geomShader = loadFile(gshPath);
	std::string vertShader = loadFile(vshPath);
	std::string fragShader = loadFile(fshPath);

	shader->initialize(geomShader, vertShader, fragShader);

	if (shader->GetProgram() == 0)
	{
		Log::Warn("[ShaderManager] Shader program loading failed, loading default!");
		shader->initialize(default_vertex_shader, default_frag_shader);
	}
	return shader;
}


std::string ShaderManager::loadFile(const std::string filePath)
{
	long fileSize = FileUtil::GetFileSize(filePath);

	const size_t blockSize = 1024;
	char *source = (char*)malloc(fileSize+1);
	if (!source)
	{
		Log::Error("[Shader] Unable to malloc %l bytes for reading", fileSize);
		return "";
	}

	std::string fullPath = "";
	fullPath.append(filePath);

	FILE *fp;
#ifdef _WIN32
	fopen_s(&fp, fullPath.c_str(), "r");
#else
	fp = fopen(fullPath.c_str(), "r");
#endif
	if (!fp)
	{
		Log::Error("[Shader] Unable to open %s for reading", fullPath.c_str());
		return "";
	}

	size_t blockBytes, readBytes = 0;
	// Read one block at a time
	while ((blockBytes = fread(source+readBytes, 1, blockSize, fp)) > 0)
	{
		readBytes += blockBytes;
	}

	/* close the file and null terminate the string */
	fclose(fp);
	
	source[readBytes] = '\0';

	std::string data(source);

	delete source;

	return data;
}


