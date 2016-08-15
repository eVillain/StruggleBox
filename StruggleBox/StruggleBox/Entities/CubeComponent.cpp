#include "CubeComponent.h"
#include "VoxelFactory.h"
#include "InstancedMesh.h"
#include "EntityManager.h"
#include "Entity.h"
#include "Log.h"

CubeComponent::CubeComponent(
	const int ownerID,
	const std::string& objectName,
	std::shared_ptr<EntityManager> manager,
	std::shared_ptr<VoxelFactory> voxels) :
EntityComponent(ownerID, "Cube"),
_manager(manager),
_voxels(voxels)
{
	Log::Debug("[CubeComponent] constructor, instance at: %p", this);
    _instanceID = -1;
    Entity* _owner = _manager->getEntity(_ownerID);
    if ( !_owner->HasAttribute("objectFile") ) {
        if ( objectName.empty() ) {
            _owner->GetAttributeDataPtr<std::string>("objectFile") = "UnknownObject";
        } else { // Save the filename into an entity attribute
            _owner->GetAttributeDataPtr<std::string>("objectFile") = objectName;
        }
        _owner->GetAttributeDataPtr<glm::vec3>("scale") = glm::vec3(0.1f);
    }
    loadObject();
}

CubeComponent::~CubeComponent()
{
    Log::Debug("[CubeComponent] destructor, instance at: %p, owner %i", this, _ownerID);
    unloadObject();
    _instanceID = -1;
}

void CubeComponent::loadObject()
{
	Entity* _owner = _manager->getEntity(_ownerID);
	if (!_owner->HasAttribute("objectFile"))
	{
		Log::Error("[CubeComponent] cant load object for owner %i, no object file attribute", _ownerID);
		return;
	}
	const std::string objFileName = _owner->GetAttributeDataPtr<std::string>("objectFile");
	printf("CubeComp: had object file name %s\n", objFileName.c_str());
	_mesh = _voxels->getMesh(objFileName);

	if (_instanceID == -1 && _mesh)
	{
		glm::vec3 position = _owner->GetAttributeDataPtr<glm::vec3>("position");
		_instanceID = _mesh->addInstance();
		_owner->GetAttributeDataPtr<int>("instanceID") = _instanceID;
		printf("CubeComp: new instance, id %i\n", _instanceID);
	}
	// Bounding box
	glm::vec3& bb = _owner->GetAttributeDataPtr<glm::vec3>("bb");
	bb = _voxels->getVoxels(objFileName)->getVolume(DEFAULT_VOXEL_MESHING_WIDTH);
}

void CubeComponent::unloadObject()
{
	if (_instanceID == -1)
		return;

	Entity* _owner = _manager->getEntity(_ownerID);
	_mesh->removeInstance(_instanceID);
	_instanceID = -1;
	_owner->GetAttributeDataPtr<int>("instanceID") = -1;
	_mesh = nullptr;
}

void CubeComponent::update(const double delta )
{
    if ( _instanceID != -1 )
	{
        Entity* _owner = _manager->getEntity(_ownerID);
        _mesh->setPosition(_owner->GetAttributeDataPtr<glm::vec3>("position"), _instanceID);
        _mesh->setRotation(_owner->GetAttributeDataPtr<glm::quat>("rotation"), _instanceID);
        _mesh->setScale(_owner->GetAttributeDataPtr<glm::vec3>("scale"), _instanceID);
    }
}

