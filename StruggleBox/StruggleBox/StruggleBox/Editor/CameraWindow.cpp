#include "CameraWindow.h"

#include "Camera3D.h"

#include "GUI.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "SpriteNode.h"
#include "LayoutNode.h"
#include "ValueEditNode.h"
#include "MathUtils.h"

const glm::vec2 CameraWindow::WINDOW_SIZE = glm::vec2(260.f, 600.f);

CameraWindow::CameraWindow(const GUI& gui, Camera3D& camera)
	: WindowNode(gui, WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, "Camera")
	, m_camera(camera)
{
	Log::Debug("[CameraWindow] constructor, instance at %p", this);

	m_layoutNode = m_gui.createCustomNode<LayoutNode>();
	m_layoutNode->setContentSize(m_contentNode->getContentSize());
	m_layoutNode->setLayoutType(LayoutType::Column);
	m_layoutNode->setAlignmentV(LayoutAlignmentV::Top);
	addChild(m_layoutNode);

	refresh();
}

void CameraWindow::refresh()
{
	const auto& children = m_layoutNode->getChildren();
	m_gui.destroyNodes(children);
	m_layoutNode->removeAllChildren();

	m_layoutNode->setContentSize(m_contentNode->getContentSize());
	
	const glm::vec2 SMALL_VALUE_SIZE = glm::vec2(m_contentSize.x - 4, 20.f);
	const glm::vec2 LARGE_VALUE_SIZE = glm::vec2(m_contentSize.x - 4, 80.f);

	ValueEditNode<bool>* autoRotate = m_gui.createCustomNode<ValueEditNode<bool>>(m_gui);
	autoRotate->setContentSize(SMALL_VALUE_SIZE);
	autoRotate->setValue("Auto-Rotate", m_camera.m_autoRotate, false, true);
	m_layoutNode->addChild(autoRotate);

	ValueEditNode<bool>* thirdPerson = m_gui.createCustomNode<ValueEditNode<bool>>(m_gui);
	thirdPerson->setContentSize(SMALL_VALUE_SIZE);
	thirdPerson->setValue("ThirdPerson", m_camera.m_thirdPerson, false, true);
	m_layoutNode->addChild(thirdPerson);

	ValueEditNode<bool>* elasticMovement = m_gui.createCustomNode<ValueEditNode<bool>>(m_gui);
	elasticMovement->setContentSize(SMALL_VALUE_SIZE);
	elasticMovement->setValue("Elastic", m_camera.m_elasticMovement, false, true);
	m_layoutNode->addChild(elasticMovement);

	//ValueEditNode<bool>* physicsClip = m_gui.createCustomNode<ValueEditNode<bool>>(m_gui);
	//physicsClip->setContentSize(SMALL_VALUE_SIZE);
	//physicsClip->setValue("Physics", m_camera.m_physicsClip, false, true);
	//m_layoutNode->addChild(physicsClip);

	//ValueEditNode<float>* elasticity = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	//elasticity->setContentSize(SMALL_VALUE_SIZE);
	//elasticity->setValue("Elasticity", m_camera.m_elasticity, 0.f, 100.f);
	//m_layoutNode->addChild(elasticity);

	ValueEditNode<float>* distance = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	distance->setContentSize(SMALL_VALUE_SIZE);
	distance->setValue("Distance", m_camera.m_distance, 0.01f, 10.f);
	m_layoutNode->addChild(distance);

	//ValueEditNode<float>* maxDistance = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	//maxDistance->setContentSize(SMALL_VALUE_SIZE);
	//maxDistance->setValue("MaxDistance", m_camera.m_maxDistance, 0.5f, 20.f);
	//m_layoutNode->addChild(maxDistance);

	//ValueEditNode<float>* height = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	//height->setContentSize(SMALL_VALUE_SIZE);
	//height->setValue("Height", m_camera.height, 0.f, 2.f);
	//m_layoutNode->addChild(height);

	//ValueEditNode<float>* movementSpeedFactor = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	//movementSpeedFactor->setContentSize(SMALL_VALUE_SIZE);
	//movementSpeedFactor->setValue("MovementSpeedFactor", m_camera.movementSpeedFactor, 1.f, 100.f);
	//m_layoutNode->addChild(movementSpeedFactor);

	ValueEditNode<float>* fieldOfView = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	fieldOfView->setContentSize(SMALL_VALUE_SIZE);
	fieldOfView->setValue("FieldOfView", m_camera.m_fieldOfView, 0.f, 2.f);
	m_layoutNode->addChild(fieldOfView);

	ValueEditNode<float>* nearDepth = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	nearDepth->setContentSize(SMALL_VALUE_SIZE);
	nearDepth->setValue("NearDepth", m_camera.m_nearDepth, -100.f, 1.f);
	m_layoutNode->addChild(nearDepth);

	ValueEditNode<float>* farDepth = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	farDepth->setContentSize(SMALL_VALUE_SIZE);
	farDepth->setValue("FarDepth", m_camera.m_farDepth, 10.f, 1000.f);
	m_layoutNode->addChild(farDepth);

	//ValueEditNode<float>* focalDepth = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	//focalDepth->setContentSize(SMALL_VALUE_SIZE);
	//focalDepth->setValue("FocalDepth", m_camera.m_focalDepth, 0.f, 20.f);
	//m_layoutNode->addChild(focalDepth);

	//ValueEditNode<float>* focalLength = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	//focalLength->setContentSize(SMALL_VALUE_SIZE);
	//focalLength->setValue("FocalLength", m_camera.m_focalLength, 0.f, 10.f);
	//m_layoutNode->addChild(focalLength);

	//ValueEditNode<float>* fStop = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	//fStop->setContentSize(SMALL_VALUE_SIZE);
	//fStop->setValue("f-Stop", m_camera.m_fStop, 0.1f, 22.f);
	//m_layoutNode->addChild(fStop);

	//ValueEditNode<float>* exposure = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	//exposure->setContentSize(SMALL_VALUE_SIZE);
	//exposure->setValue("Exposure", m_camera.m_exposure, 0.f, 2.f);
	//m_layoutNode->addChild(exposure);

	ValueEditNode<glm::vec3>* rot = m_gui.createCustomNode<ValueEditNode<glm::vec3>>(m_gui);
	rot->setContentSize(LARGE_VALUE_SIZE);
	rot->setValue("Rotation", m_camera.m_rotation, -glm::vec3(MathUtils::PI), glm::vec3(MathUtils::PI));
	m_layoutNode->addChild(rot);

	ValueEditNode<glm::vec3>* pos = m_gui.createCustomNode<ValueEditNode<glm::vec3>>(m_gui);
	pos->setContentSize(LARGE_VALUE_SIZE);
	pos->setValue("Position", m_camera.m_position, -glm::vec3(10.f), glm::vec3(10.f));
	m_layoutNode->addChild(pos);

	m_layoutNode->refresh();
}