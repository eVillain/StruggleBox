#include "VoxelComponent.h"
#include "VoxelFactory.h"
#include "InstancedMesh.h"
#include "EntityManager.h"
#include "Entity.h"
#include "Log.h"

VoxelComponent::VoxelComponent(const int ownerID, const std::string& objectName, EntityManager& manager, VoxelFactory& voxels) 
	: EntityComponent(ownerID, "Cube")
	, _manager(manager)
	, _voxels(voxels)
	, _mesh(nullptr)
	, _instanceID(0)
{
	Log::Debug("[VoxelComponent] constructor, instance at: %p", this);
    Entity* _owner = _manager.getEntity(_ownerID);
    if ( !_owner->HasAttribute("objectFile") ) 
	{
        if ( objectName.empty() ) 
		{
			_owner->GetAttributeDataPtr<std::string>("objectFile") = "UnknownObject";
        }
		else 
		{
			// Save the filename into an entity attribute
            _owner->GetAttributeDataPtr<std::string>("objectFile") = objectName;
        }
        _owner->GetAttributeDataPtr<glm::vec3>("scale") = glm::vec3(0.1f);
    }
    loadObject();
}

VoxelComponent::~VoxelComponent()
{
    Log::Debug("[VoxelComponent] destructor, instance at: %p, owner %i", this, _ownerID);
    unloadObject();
}

void VoxelComponent::loadObject()
{
	if (_instanceID && _mesh)
	{
		return;
	}
	Entity* _owner = _manager.getEntity(_ownerID);
	if (!_owner->HasAttribute("objectFile"))
	{
		Log::Error("[VoxelComponent] cant load object for owner %i, no object file attribute", _ownerID);
		return;
	}
	const std::string objFileName = _owner->GetAttributeDataPtr<std::string>("objectFile");
	_mesh = _voxels.getMesh(objFileName);

	if (_instanceID == 0 && _mesh)
	{
		glm::vec3 position = _owner->GetAttributeDataPtr<glm::vec3>("position");
		_instanceID = _mesh->addInstance();
		_owner->GetAttributeDataPtr<int>("instanceID") = _instanceID;
		Log::Debug("[VoxelComponent] new voxel instance, id %i for file %s", _instanceID, objFileName.c_str());

		// Bounding box
		glm::vec3& bb = _owner->GetAttributeDataPtr<glm::vec3>("bb");
		bb = _voxels.getVoxels(objFileName)->getVolume(DEFAULT_VOXEL_MESHING_WIDTH);
	}
}

void VoxelComponent::unloadObject()
{
	if (_instanceID == 0)
		return;

	Entity* _owner = _manager.getEntity(_ownerID);
	_mesh->removeInstance(_instanceID);
	_owner->GetAttributeDataPtr<int>("instanceID") = 0;
	_instanceID = 0;
	_mesh = nullptr;
}

void VoxelComponent::update(const double delta )
{
    if (_instanceID != 0 && _mesh)
	{
        Entity* _owner = _manager.getEntity(_ownerID);
        _mesh->setPosition(_owner->GetAttributeDataPtr<glm::vec3>("position"), _instanceID);
        _mesh->setRotation(_owner->GetAttributeDataPtr<glm::quat>("rotation"), _instanceID);
        _mesh->setScale(_owner->GetAttributeDataPtr<glm::vec3>("scale"), _instanceID);
    }
}

