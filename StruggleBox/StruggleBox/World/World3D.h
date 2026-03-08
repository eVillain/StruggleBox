#pragma once

#include "Coord.h"
#include "Entity.h"
#include "EntityManager.h"
#include "GFXHelpers.h"
#include "ItemComponent.h"
#include "Physics.h"
#include "PhysicsCube.h"
#include "Particles.h"
#include "VoxelCache.h"
#include "Lighting3DDeferred.h"
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <mutex>

class Allocator;
class Options;
class Shader;
class VoxelRenderer;

class World3D
{
public:
    World3D(
        Allocator& allocator,
        Injector& injector,
        VoxelRenderer& renderer,
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
    
    Physics& getPhysics() { return m_physics; }
    VoxelCache& getVoxelFactory() { return m_voxelCache; }
    EntityManager& getEntityManager() { return m_entityMan; }

    static bool paused;                         // Used to switch off physics updates and freeze world
    static bool physicsEnabled;                 // Enable bullet physics engine
    static float worldTimeScale;                // 1.0 for normal speed, smaller for slo-mo
    
    std::string worldName;                      // Name of world, folder used for data streaming

    LightInstance playerLight;                       // light at player pos
   
    bool m_refreshPhysics;                        // DEBUG: Do a full refresh on all physics vertices
    
    std::vector<glm::vec3> wantedTrees;         // Positions for wanted trees
    
    // Entities
    EntityID m_playerID;

private:
    Allocator& m_allocator;
    VoxelRenderer& m_renderer;
	Options& m_options;

	Particles m_particles;
    Physics m_physics;
    VoxelCache m_voxelCache;
    EntityManager m_entityMan;

    ShaderID m_voxelInstancesShaderID;

    std::vector<PhysicsCube*> dynamicCubes;        // Dynamic cubes with physics
    std::vector<PhysicsCube*> staticCubes;       // Static cubes with physics
    std::vector<int> objectLabels;              // Labeling of nearby objects

    double m_gameTime;

	LightInstance m_lights[4];

    //CubeInstanceColor* m_lightCubes;

    struct TerrainChunk {
        VoxelData* voxels;
        DrawDataID drawDataID;
        size_t vertexCount;
        uint32_t physicsShapeID;
        uint32_t physicsBodyID;
    };

    std::map<Coord3D, TerrainChunk> m_chunks;

    void updateChunks();

};
