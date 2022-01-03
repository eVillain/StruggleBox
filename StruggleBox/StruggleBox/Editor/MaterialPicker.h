#pragma once

#include "MaterialData.h"
#include "InteractableNode.h"
#include <functional>
#include <string>

class GUI;
class ButtonNode;
class LabelNode;
class ScrollerNode;
class SliderNode;
class SpriteNode;
class Renderer;

class MaterialPicker : public InteractableNode
{
public:
	MaterialPicker(
		const GUI& gui,
		Renderer& renderer,
		MaterialData& materials,
		const glm::vec2& size);

	bool onPress(const glm::vec2&) override;
	bool onRelease(const glm::vec2& relativeCursorPosition) override;
	void onHighlight(bool inside, const glm::vec2&) override;

	void setCallback(const std::function<void(uint8_t)>& callback) { m_callback = callback; }

	void setMaterialID(uint8_t materialID);
	uint8_t getCurrentID() const { return m_currentID; }

private:
	const GUI& m_gui;
	Renderer& m_renderer;
	MaterialData& m_materials;

	SpriteNode* m_materialSprite;
	SpriteNode* m_materialSelectorSprite;

	uint8_t m_currentID;

	std::function<void(uint8_t)> m_callback;


	void setup();
	void remove();
	void refresh();
};

