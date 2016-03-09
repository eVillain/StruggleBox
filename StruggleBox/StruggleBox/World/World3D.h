#ifndef NGN_WORLD3D_H
#define NGN_WORLD3D_H

#include <map>
#include <queue>
#include <string>
#include <vector>
#include <mutex>

#include "Entity.h"
#include "ItemComponent.h"
#include "DynaCube.h"
#include "StaticCube.h"
#include "GFXHelpers.h"
#include "Coord.h"

class Locator;
class Renderer;
class Cubeject;
class Shader;
class Light3D;

class World3D
{
public:
    
    World3D(const std::string newName,
            const int newSeed,
            Locator& locator);
    ~World3D();
    
    void Update( double delta );
    void UpdateLabels(Renderer* renderer);
    void ClearLabels();
    
    void Draw(Renderer* renderer);
    void DrawObjects(Renderer* renderer);
    
    const int Spawn( const std::string filePath, const std::string fileName );
    const int SpawnItem( const ItemType type, const std::string object, const glm::vec3 pos, const glm::quat rot=glm::quat() );

    const int AddPlayer( const glm::vec3 pos );
    const int AddSkeleton( const glm::vec3 pos );
    const int AddHuman( const glm::vec3 pos );
    void AddDecor(const std::string object,
                  const glm::vec3 pos,
                  const glm::quat rot = glm::quat() );
    
    // Dynamic physics cubes testing
    DynaCube* AddDynaCube( const btVector3 & pos, const btVector3 & halfSize, const Color& col );
    void RemoveDynaCube( DynaCube* cube );

    // Objects
    Cubeject* LoadObject( const std::string fileName );
    unsigned int AddObject( const std::string objectName, const glm::vec3 position, const glm::vec3 scale = glm::vec3(1.0f) );
    void RemoveObject( InstanceData* object );
    void RemoveObject( unsigned int objectID );
    Cubeject* GetObject( const std::string objectName );

    void Explosion( const glm::vec3 position, const float radius, const float force );
    
    
    static bool paused;                         // Used to switch off physics updates and freeze world
    static bool physicsEnabled;                 // Enable bullet physics engine
    static float worldTimeScale;                // 1.0 for normal speed, smaller for slo-mo
    
    std::string worldName;                      // Name of world, folder used for data streaming
    int seed;                                   // Random number which seeds cube generation
    int waterLevel;                             // Height of sea level in meters
    float waterWaveHeight;                      // Height of waves in meters
    
    int numContacts;                            // Number of physics contacts generated this frame
    std::map<std::string, Cubeject*> cubejects; // Map of Cubejects sorted by their name
    
    Light3D* sunLight;                          // Pointer to a sun light data object
    Light3D* playerLight;                       // Pointer to light at player pos
    
    class Physics * worldPhysics;               // Pointer to physics interface
    bool refreshPhysics;                        // DEBUG: Do a full refresh on all physics vertices
    
    std::vector<glm::vec3> wantedTrees; // Positions for wanted trees
    
    // Entities
    class EntityManager* entityMan;
    int playerID;
    Coord3D playerCoord;    // TEMPORARY

private:
    Locator& _locator;
    std::vector<DynaCube*> dynamicCubes;        // Dynamic cubes with physics
    std::vector<StaticCube*> staticCubes;       // Static cubes with physics
    std::vector<int> objectLabels;              // Labeling of nearby objects

};


#endif
