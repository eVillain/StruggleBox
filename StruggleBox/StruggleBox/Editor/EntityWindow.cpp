#include "EntityWindow.h"

#include "Renderer.h"
#include "EntityManager.h"
#include "GUI.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "SpriteNode.h"
#include "ScrollerNode.h"
#include "OptionsWindow.h"
#include "ValueEditNode.h"

#include "PhysicsComponent.h"

const glm::vec2 EntityWindow::WINDOW_SIZE = glm::vec2(420.f, 600.f);

EntityWindow::EntityWindow(
	const GUI& gui,
	Renderer& renderer,
	EntityManager& entityManager)
	: TabWindow(gui, WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, "Entity")
	, m_renderer(renderer)
	, m_entityManager(entityManager)
	, m_entity(nullptr)
	, m_entityID(0)
	, m_scrollerNode(nullptr)
{
	Log::Debug("[EntityWindow] constructor, instance at %p", this);

	m_scrollerNode = m_gui.createScrollerNode(GUI::RECT_DEFAULT);
	m_scrollerNode->setContentSize(glm::vec2(m_contentNode->getContentSize().x - 4.f, m_contentNode->getContentSize().y - 34.f));
	m_scrollerNode->setPosition(glm::vec3(2.f, 2.f, 1.f));
	addChild(m_scrollerNode);
}

void EntityWindow::setEntity(Entity* entity, EntityID entityID)
{
	m_entityID = entityID;
	m_entity = entity;

	m_availableTabs.clear();
	if (m_entity)
	{
		m_availableTabs = { "Attributes", "Components" };
		for (const std::string& compFamily : EntityManager::ENTITY_COMPONENT_FAMILY_NAMES)
		{
			EntityComponent* component = m_entityManager.getComponent(m_entityID, compFamily);
			if (!component)
			{
				continue;
			}
			m_availableTabs.push_back(compFamily);
		}
	}

	refreshTabs();
}

void EntityWindow::setContentSize(const glm::vec2& size)
{
	TabWindow::setContentSize(size);
	m_scrollerNode->setContentSize(glm::vec2(m_contentNode->getContentSize().x, m_contentNode->getContentSize().y - 32.f));
}

const std::vector<std::string>& EntityWindow::getTabTitles()
{
	if (!m_availableTabs.empty())
	{
		return m_availableTabs;
	}

	static const std::vector<std::string> BASE_TABS = {
		"Attributes", "Components"
	};

	return BASE_TABS;
}

void EntityWindow::setTabContent(const uint32_t tab)
{
	std::vector<Node*> nodes;
	if (tab == 0)
	{
		showAttributesTab();
	}
	else if (tab == 1)
	{
		showComponentsTab();
	}
	else if (m_availableTabs.size() > tab)
	{
		showTabForComponent(m_availableTabs.at(tab));
	}
}

void EntityWindow::showAttributesTab()
{
	std::vector<Node*> nodes;

	std::map<const std::string, Attribute*>& attributes = m_entity->GetAttributes();
	for (const auto& pair : attributes)
	{
		const glm::vec2 SMALL_COMPONENT_SIZE = glm::vec2(m_contentNode->getContentSize().x - 8.f, 24.f);
		const glm::vec2 LARGE_COMPONENT_SIZE = glm::vec2(m_contentNode->getContentSize().x - 8.f, 76.f);
		const std::string& name = pair.first;
		Attribute* attribute = pair.second;
		if (attribute->IsType<bool>())
		{
			ValueEditNode<bool>* node = m_gui.createCustomNode<ValueEditNode<bool>>(m_gui);
			node->setContentSize(SMALL_COMPONENT_SIZE);
			node->setValue(name, attribute->as<bool>(), false, true);
			nodes.push_back(node);
		}
		else if (attribute->IsType<int>())
		{
			ValueEditNode<int>* node = m_gui.createCustomNode<ValueEditNode<int>>(m_gui);
			node->setContentSize(SMALL_COMPONENT_SIZE);
			node->setValue(name, attribute->as<int>(), 0, 100);
			nodes.push_back(node);
		}
		else if (attribute->IsType<float>())
		{
			ValueEditNode<float>* node = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
			node->setContentSize(SMALL_COMPONENT_SIZE);
			node->setValue(name, attribute->as<float>(), 0.f, 100.f);
			nodes.push_back(node);
		}
		else if (attribute->IsType<std::string>())
		{
			ValueEditNode<std::string>* node = m_gui.createCustomNode<ValueEditNode<std::string>>(m_gui);
			node->setContentSize(SMALL_COMPONENT_SIZE);
			node->setValue(name, attribute->as<std::string>(), "", "");
			nodes.push_back(node);
		}
		else if (attribute->IsType<glm::vec3>())
		{
			ValueEditNode<glm::vec3>* node = m_gui.createCustomNode<ValueEditNode<glm::vec3>>(m_gui);
			node->setContentSize(LARGE_COMPONENT_SIZE);
			node->setValue(name, attribute->as<glm::vec3>(), glm::vec3(-10.f), glm::vec3(10.f));
			nodes.push_back(node);
		}
	}

	m_scrollerNode->setContent(nodes, ScrollStrategy::SCROLL_TO_TOP);
}

void EntityWindow::showComponentsTab()
{
	std::vector<Node*> nodes;

	for (const std::string& compFamily : EntityManager::ENTITY_COMPONENT_FAMILY_NAMES)
	{
		EntityComponent* component = m_entityManager.getComponent(m_entityID, compFamily);
		if (!component)
		{
			ButtonNode* createCompButton = m_gui.createDefaultButton(glm::vec2(m_contentNode->getContentSize().x - 8.f, 24.f), "Add " + compFamily + "Component");
			createCompButton->setCallback([this, compFamily](bool) {
				m_entityManager.addComponent(m_entityID, compFamily);
				});
			nodes.push_back(createCompButton);
			continue;
		}

		ButtonNode* removeCompButton = m_gui.createDefaultButton(glm::vec2(m_contentNode->getContentSize().x - 8.f, 24.f), "Remove " + compFamily + "Component");
		removeCompButton->setCallback([this, component](bool) {
			m_entityManager.removeComponent(m_entityID, component);
			});
		nodes.push_back(removeCompButton);
	}

	m_scrollerNode->setContent(nodes, ScrollStrategy::SCROLL_TO_TOP);
}

void EntityWindow::showTabForComponent(const std::string& componentFamily)
{
	EntityComponent* component = m_entityManager.getComponent(m_entityID, componentFamily);
	// TODO: Show individual component data here
	std::vector<Node*> nodes;
	if (componentFamily == "Physics")
	{
		PhysicsComponent* pComp = (PhysicsComponent*)component;
		ButtonNode* activateButton = m_gui.createDefaultButton(glm::vec2(m_contentNode->getContentSize().x - 8.f, 24.f), "Activate");
		activateButton->setToggleable(true);
		activateButton->setCallback([this, pComp](bool state) {
			pComp->setPhysicsMode(state ? PhysicsMode::Physics_Cube_AABBs : PhysicsMode::Physics_Off, false, false);
			});
		nodes.push_back(activateButton);
	}

	m_scrollerNode->setContent(nodes, ScrollStrategy::SCROLL_TO_TOP);
}
