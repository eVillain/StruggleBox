#pragma once

#include "TabWindow.h"
#include "Entity.h"
#include <functional>
#include <memory>

class ButtonNode;
class SpriteNode;
class EntityManager;
class ScrollerNode;

class EntityWindow : public TabWindow
{
public:
	EntityWindow(
		const GUI& gui,
		Renderer& renderer,
		EntityManager& entityManager);

	void setEntity(Entity* entity, EntityID entityID);

	void setContentSize(const glm::vec2& size) override;

	const std::vector<std::string>& getTabTitles() override;
	void setTabContent(const uint32_t tab) override;

private:
	static const glm::vec2 WINDOW_SIZE;

	Renderer& m_renderer;
	EntityManager& m_entityManager;
	Entity* m_entity;
	EntityID m_entityID;

	ScrollerNode* m_scrollerNode;
	std::vector<std::string> m_availableTabs;

	void showAttributesTab();
	void showComponentsTab();
	void showTabForComponent(const std::string& componentFamily);
};
