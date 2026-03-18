#include "ComputeTestScene.h"

#include "FileUtil.h"
#include "GLUtils.h"
#include "Input.h"
#include "MathUtils.h"
#include "Options.h"
#include "Renderer2D.h"
#include "RenderCore.h"
#include "SceneManager.h"
#include "Shader.h"
#include "Texture.h"

GLubyte block[8 * 8 * 8] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1,

	1, 0, 0, 0, 0, 0, 0, 1,
	0, 1, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 0, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 1,

	1, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 1,

	1, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 1,

	1, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 1,

	1, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 1,

	1, 0, 0, 0, 0, 0, 0, 1,
	0, 1, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 0, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 1,

	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};


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
	, m_blockTextureID(0)
	, m_frameBufferTextureID(0)
	, m_computeShaderID(0)
{
}

ComputeTestScene::~ComputeTestScene()
{
	m_renderCore.removeTexture(m_blockTextureID);
	m_renderCore.removeTexture(m_frameBufferTextureID);

}

void ComputeTestScene::Initialize()
{
	GUIScene::Initialize();
	// 	glTexImage3D(GL_TEXTURE_3D, 0, format, width, height, depth, border, format, type, pixels);
	GLuint blockTexHandle = GLUtils::createTexture3D(8, 8, 8, 0, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false, block);
	Texture* blockTexture = CUSTOM_NEW(Texture, m_renderCore.getAllocator())(blockTexHandle, 8, 8, 8, GL_RED, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, 0);
	m_blockTextureID = m_renderCore.getTextureCache().addTexture(blockTexture, "BlockTestTexture");

	const glm::ivec2 resolution = m_renderCore.getRenderResolution();
	m_camera.setViewSize(resolution);
	m_camera.setPosition(glm::vec3(3.f, 2.f, 7.f));
	m_camera.setTargetPosition(glm::vec3(3.f, 2.f, 7.f));
	m_camera.setRotation(glm::vec3(0, 0.5, 0));
	m_camera.setTargetRotation(glm::vec3(0, 0.5, 0));
	GLuint textureHandle = GLUtils::GenerateTextureRGBAF(resolution.x, resolution.y);
	Texture* texture = CUSTOM_NEW(Texture, m_renderCore.getAllocator())(textureHandle, resolution.x, resolution.y, 0, GL_RGBA, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, 0);
	m_frameBufferTextureID = m_renderCore.getTextureCache().addTexture(texture, "ComputeTestTexture");
	m_computeShaderID = m_renderCore.getShaderID("c_compute_test.csh");

	m_renderer.getDefaultCamera().setPosition(glm::vec2(resolution.x / 2, resolution.y / 2));
	m_renderer.getDefaultCamera().setTargetPosition(glm::vec2(resolution.x / 2, resolution.y / 2));
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
	float deadZone = 0.35f;
	if (fabsf(m_inputMove.x) + fabsf(m_inputMove.y) < deadZone) m_inputMove = glm::vec2();
	if (fabsf(m_inputRotate.x) + fabsf(m_inputRotate.y) < deadZone) m_inputRotate = glm::vec2();
	if (!m_camera.getThirdPerson())
	{
		glm::vec3 movement = m_camera.getMovement();
		movement.x = m_inputMove.x;
		movement.z = m_inputMove.y;
		m_camera.setMovement(movement);
	}
	float joySensitivity = 2.0f;
	float rotationX = -m_inputRotate.x * joySensitivity;
	float rotationY = -m_inputRotate.y * joySensitivity;
	m_camera.rotate(rotationX, rotationY);

	GUIScene::Update(delta);
	m_renderer.update(delta);
	m_camera.update(delta);
}

void ComputeTestScene::Draw()
{
	GUIScene::Draw();
	
	const glm::ivec2 resolution = m_renderCore.getRenderResolution();

	glm::vec3 ray00, ray10, ray01, ray11;
	m_camera.getFrustumCorners(ray00, ray10, ray01, ray11);

	const Texture* fbTexture = m_renderCore.getTextureByID(m_frameBufferTextureID);
	const Texture* blockTexture = m_renderCore.getTextureByID(m_blockTextureID);
	const Shader* computeShader = m_renderCore.getShaderByID(m_computeShaderID);
	computeShader->begin();
	GLint localWorkGroupSize[3];
	glGetProgramiv(computeShader->GetProgram(), GL_COMPUTE_WORK_GROUP_SIZE, localWorkGroupSize);
	const uint32_t workGroupsX = MathUtils::round_up_to_power_of_2((uint32_t)resolution.x / localWorkGroupSize[0]);
	const uint32_t workGroupsY = MathUtils::round_up_to_power_of_2((uint32_t)resolution.y / localWorkGroupSize[1]);

	computeShader->setUniform3fv("eye", m_camera.getPosition());
	computeShader->setUniform3fv("ray00", ray00);
	computeShader->setUniform3fv("ray10", ray10);
	computeShader->setUniform3fv("ray01", ray01);
	computeShader->setUniform3fv("ray11", ray11);
	glBindImageTexture(0, fbTexture->getGLTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(1, blockTexture->getGLTextureID(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_R8UI);
	glDispatchCompute(workGroupsX, workGroupsY, 1);
	// make sure writing to image has finished before read
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_renderer.drawRectTextured(Rect2D(0, 0, resolution.x, resolution.y), Rect2D(0, 0, 1, 1), m_frameBufferTextureID, 0.f);

	m_renderer.flush();
}

bool ComputeTestScene::OnMouse(const glm::ivec2& coord)
{
	if (GUIScene::OnMouse(coord))
	{
		return true;
	}

	const int windowWidth = m_options.getOption<int>("r_resolutionX");
	const int windowHeight = m_options.getOption<int>("r_resolutionY");
	const int midWindowX = windowWidth / 2;     // Middle of the window horizontally
	const int midWindowY = windowHeight / 2;    // Middle of the window vertically
	if (m_options.getOption<bool>("r_grabCursor"))
	{
		float mouseSensitivity = 0.003f;
		float rotationX = (midWindowX - coord.x) * mouseSensitivity;
		float rotationY = (midWindowY - coord.y) * mouseSensitivity;

		if (m_camera.getThirdPerson())
		{
			rotationX *= -1.0f;
			rotationY *= -1.0f;
		}

		m_camera.rotate(rotationX, rotationY);

		// Reset the mouse position to the centre of the window each frame
		m_input.MoveCursor(glm::ivec2(midWindowX, midWindowY));
		//m_cursor.posScrn = glm::vec2(midWindowX, midWindowY);
		return true;
	}
	else
	{
		//m_cursor.posScrn = glm::vec2(coord.x, windowHeight - coord.y);
	}
	return false;
}


bool ComputeTestScene::OnEvent(const InputEvent event, const float amount)
{
	if (GUIScene::OnEvent(event, amount))
	{
		return true;
	}
	if (amount == -1.0f)
	{
		if (event == InputEvent::Edit_Grab_Cursor)
		{
			bool& grabCursor = m_options.getOption<bool>("r_grabCursor");
			grabCursor = !grabCursor;
			//SDL_ShowCursor(!grabCursor);
			return true;
		}
	}
	if (event == InputEvent::Move_Forward)
	{
		m_inputMove.y += amount;
	}
	else if (event == InputEvent::Move_Backward)
	{
		m_inputMove.y += -amount;
	}
	else if (event == InputEvent::Move_Left)
	{
		m_inputMove.x += -amount;
	}
	else if (event == InputEvent::Move_Right)
	{
		m_inputMove.x += amount;
	}

	if (event == InputEvent::Back && amount < 0.f)
	{
		m_sceneManager.DropActiveScene();
		return true;
	}
	return false;
}
