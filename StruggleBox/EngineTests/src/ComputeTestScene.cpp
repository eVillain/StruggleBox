#include "ComputeTestScene.h"

#include "FileUtil.h"
#include "GLUtils.h"
#include "Renderer2D.h"
#include "RenderCore.h"
#include "SceneManager.h"
#include "Shader.h"
#include "Texture2D.h"

const unsigned int TEXTURE_WIDTH = 512, TEXTURE_HEIGHT = 512;

ComputeTestScene::ComputeTestScene(
	Allocator& allocator,
	Renderer2D& renderer,
	RenderCore& renderCore,
	SceneManager& sceneManager,
	Input& input,
	OSWindow& window,
	Options& options,
	StatTracker& statTracker)
	: GUIScene("ComputeTestScene", allocator, renderCore, input, window, options, statTracker)
	, m_renderer(renderer)
	, m_renderCore(renderCore)
	, m_sceneManager(sceneManager)
	, m_textureID(0)
	, m_computeShaderID(0)
{
}

ComputeTestScene::~ComputeTestScene()
{
	m_renderCore.removeTexture(m_textureID);

}

void ComputeTestScene::Initialize()
{
	GUIScene::Initialize();

	GLuint textureHandle = GLUtils::GenerateTextureRGBAF(TEXTURE_WIDTH, TEXTURE_HEIGHT);
	Texture2D* texture = CUSTOM_NEW(Texture2D, m_renderCore.getAllocator())(textureHandle, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, 0);
	m_textureID = m_renderCore.getTextureCache().addTexture(texture, "ComputeTestTexture");
	m_computeShaderID = m_renderCore.getShaderID("c_compute_test.csh");
}

void ComputeTestScene::ReInitialize()
{
	GUIScene::ReInitialize();
}

void ComputeTestScene::Pause()
{
	GUIScene::Pause();
}

void ComputeTestScene::Resume()
{
	GUIScene::Resume();
}

void ComputeTestScene::Update(const double delta)
{
	GUIScene::Update(delta);
	m_renderer.update(delta);
}

void ComputeTestScene::Draw()
{
	GUIScene::Draw();

	const Texture2D* texture = m_renderCore.getTextureByID(m_textureID);
	const Shader* computeShader = m_renderCore.getShaderByID(m_computeShaderID);
	computeShader->begin();
	glBindImageTexture(0, texture->getGLTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
	// make sure writing to image has finished before read
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//ImpostorVertexData* impostorVerts = m_renderer.bufferImpostorPoints(1, m_textureID);
	//impostorVerts[0] = { glm::vec3(100, 100, 0), TEXTURE_WIDTH, COLOR_WHITE };

	const glm::ivec2 resolution = m_renderCore.getRenderResolution();
	m_renderer.drawGrid(20.f, Rect2D(0, 0, resolution.x, resolution.y), 0, COLOR_PURPLE, 0.f);
	//m_renderer.drawRectColor(Rect2D(10, 10, 200, 200), COLOR_BLUE, COLOR_RED, 0);
	//m_renderer.drawCircleColor(glm::vec2(500, 500), 0.f, 200.f, COLOR_RED, COLOR_BLUE, 0, 8);
	//m_renderer.drawRingColor(glm::vec2(0, 0), 100.f, 200.f, 32, COLOR_GREEN, COLOR_PURPLE, 0);
	m_renderer.drawRectTextured(Rect2D(300, 0, 256, 256), Rect2D(0, 0, 1, 1), m_textureID, 0.f);

	m_renderer.flush();
}

bool ComputeTestScene::OnEvent(const InputEvent event, const float amount)
{
	if (GUIScene::OnEvent(event, amount))
	{
		return true;
	}
	if (event == InputEvent::Back && amount < 0.f)
	{
		m_sceneManager.DropActiveScene();
		return true;
	}
	return false;
}
