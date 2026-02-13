#include "VoxelComponent.h"
#include "VoxelCache.h"
//#include "InstancedTriangleMesh.h"
#include "EntityManager.h"
#include "Entity.h"
#include "Log.h"

VoxelComponent::VoxelComponent(const int ownerID, const std::string& objectName, EntityManager& manager, VoxelCache& voxels)
	: EntityComponent(ownerID, "Cube")
	, _manager(manager)
	, _voxels(voxels)
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
	if (_instanceID)
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

	if (_instanceID == 0)
	{
		glm::vec3 position = _owner->GetAttributeDataPtr<glm::vec3>("position");
		_instanceID = _voxels.addInstance(objFileName);
		_owner->GetAttributeDataPtr<int>("instanceID") = _instanceID;
		Log::Debug("[VoxelComponent] new voxel instance, id %i for file %s", _instanceID, objFileName.c_str());

		// Bounding box
		glm::vec3& bb = _owner->GetAttributeDataPtr<glm::vec3>("bb");
		bb = _voxels.getVoxelData(objFileName)->getVolume(DEFAULT_VOXEL_MESHING_WIDTH);
	}
}

void VoxelComponent::unloadObject()
{
	if (_instanceID == 0)
		return;

	Entity* _owner = _manager.getEntity(_ownerID);
	const std::string objFileName = _owner->GetAttributeDataPtr<std::string>("objectFile");
	_voxels.removeInstance(objFileName, _instanceID);
	_owner->GetAttributeDataPtr<int>("instanceID") = 0;
	_instanceID = 0;
}

void VoxelComponent::update(const double delta )
{
    if (_instanceID != 0)
	{
        Entity* _owner = _manager.getEntity(_ownerID);
		const std::string objFileName = _owner->GetAttributeDataPtr<std::string>("objectFile");
		ColoredInstanceTransform3DData* instance = _voxels.getInstance(objFileName, _instanceID);
        instance->position = _owner->GetAttributeDataPtr<glm::vec3>("position");
        instance->rotation = _owner->GetAttributeDataPtr<glm::quat>("rotation");
        instance->scale = _owner->GetAttributeDataPtr<glm::vec3>("scale");
    }
}

