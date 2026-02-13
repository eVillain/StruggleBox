#pragma once

#include "Renderer2DDeferred.h"
#include "GUIScene.h"
#include "Physics2D.h"
#include "Humanoid.h"
#include "Shape2D.h"
#include <vector>

class Game2D : public GUIScene
{
public:
	Game2D(Allocator& allocator, Renderer2DDeferred& renderer, Input& input, OSWindow& window, Options& options, StatTracker& statTracker);
	~Game2D();

	void Initialize() override;
	void ReInitialize() override;
	void Release() override;

	void Pause() override;
	void Resume() override;

	void Update(const double delta) override;
	void Draw() override;

protected:
	bool OnEvent(const InputEvent event, const float amount) override;
	bool OnMouse(const glm::ivec2& coords) override;

private:
	Renderer2DDeferred& m_renderer;
	Physics2D m_physics;

	std::vector<Shape2D*> m_backgroundShapes;
	std::vector<Shape2D*> m_foregroundShapes;

	Humanoid m_player;
	glm::vec2 m_inputDirectionBuffer;
	glm::vec2 m_inputAimBuffer;

	Light2DID m_testLightID;
};

