#pragma once

#include "CoreIncludes.h"
#include "ThreadSafeQueue.h"
#include <functional>
#include <string>
#include <mutex>
#include <vector>

class Allocator;
class ThreadPool;
class Texture2D;

class TextureLoader
{
public:
	TextureLoader(Allocator& allocator, ThreadPool& threadPool);

	Texture2D* loadFromFile(const std::string& fileName, GLint wrap = GL_REPEAT, GLint minF = GL_NEAREST, GLint magF = GL_NEAREST);
	Texture2D* loadFromPNGData(const char* data);

	void asyncLoadFromFile(
		const std::string& fileName,
		const std::function<void(Texture2D*)>& callback,
		GLint wrap = GL_REPEAT,
		GLint minF = GL_NEAREST,
		GLint magF = GL_NEAREST);

	void processQueue();

private:
	struct TextureLoadPackage {
		std::string fileName;
		std::function<void(Texture2D*)> callback;
		GLint wrap;
		GLint minF;
		GLint magF;
		uint32_t width;
		uint32_t height;
		uint32_t formatGL;
		unsigned char* image_data;
		bool finished;
	};

	Allocator& m_allocator;
	ThreadPool& m_threadPool;

	std::vector<TextureLoadPackage*> m_packages;
	mutable std::mutex m_mutex;

	static void LoadPackageInThread(TextureLoadPackage* package, Allocator* allocator, std::mutex* mutex);
	static uint32_t pngColorFormatToGL(const int pngColorFormat);
};

