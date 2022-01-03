#ifndef WORLD3D_H
#define WORLD3D_H

#include <map>
#include <queue>
#include <string>
#include <vector>
#include <mutex>

#include "VoxelFactory.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Particles.h"

#include "Entity.h"
#include "ItemComponent.h"
#include "PhysicsCube.h"
#include "GFXHelpers.h"
#include "Coord.h"
#include "Light3D.h"

class Allocator;
class Renderer;
class Camera;
class Options;
class Shader;

class World3D
{
public:
    World3D(
        Allocator& allocator,
		Renderer& renderer,
		Camera& camera,
		Options& options);
    ~World3D();
    
    void Initialize();
    
    void Update( double delta );
    void UpdateLabels();
    void ClearLabels();
    
    void Draw();
    void DrawObjects();
    
    const int Spawn(std::string filePath, std::string fileName);
    const int SpawnItem(
		const ItemType type,
		std::string object,
		const glm::vec3 pos,
		const glm::quat rot=glm::quat());

    const int AddPlayer( const glm::vec3 pos );
    const int AddSkeleton( const glm::vec3 pos );
    const int AddHuman( const glm::vec3 pos );
    void AddDecor(const std::string& object,
                  const glm::vec3 pos,
                  const glm::quat rot = glm::quat(),
                  const bool isStatic = true);
    void AddParticleEntity(const std::string& fileName, const glm::vec3 pos);
    const EntityID createFireball(const glm::vec3& pos, const glm::vec3& vel, float size);

    // Dynamic physics cubes testing
    PhysicsCube* AddDynaCube(const btVector3& pos, const btVector3& size, const uint8_t materialID);
    void RemoveDynaCube( PhysicsCube* cube );

    void Explosion( const glm::vec3 position, const float radius, const float force );
    
    Physics& getPhysics() { return _physics; }
    VoxelFactory& getVoxelFactory() { return _voxels; }
    EntityManager& getEntityManager() { return _entityMan; }

    static bool paused;                         // Used to switch off physics updates and freeze world
    static bool physicsEnabled;                 // Enable bullet physics engine
    static float worldTimeScale;                // 1.0 for normal speed, smaller for slo-mo
    
    std::string worldName;                      // Name of world, folder used for data streaming

    LightInstance playerLight;                       // light at player pos
   
    bool refreshPhysics;                        // DEBUG: Do a full refresh on all physics vertices
    
    std::vector<glm::vec3> wantedTrees;         // Positions for wanted trees
    
    // Entities
    EntityID playerID;

private:
    Allocator& m_allocator;
	Renderer& _renderer;
	Camera& _camera;
	Options& _options;

	Particles _particles;
    Physics _physics;
    VoxelFactory _voxels;
    EntityManager _entityMan;

    std::vector<PhysicsCube*> dynamicCubes;        // Dynamic cubes with physics
    std::vector<PhysicsCube*> staticCubes;       // Static cubes with physics
    std::vector<int> objectLabels;              // Labeling of nearby objects

    double m_gameTime;
	//CubeInstance _floorCubes[32 * 32 * 32];
	//int _numCubes;

	LightInstance _lights[4];

    CubeInstanceColor* m_lightCubes;
};


#endif
