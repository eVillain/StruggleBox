#ifndef WORLD3D_H
#define WORLD3D_H

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
#include "Light3D.h"

class Injector;
class Renderer;
class Camera;
class EntityManager;
class Text;
class Options;
class Particles;
class Shader;

class World3D
{
public:
    World3D(
		std::shared_ptr<Injector> injector,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<EntityManager> entityMan,
		std::shared_ptr<Text> text,
		std::shared_ptr<Options> options,
		std::shared_ptr<Particles> particles,
		std::shared_ptr<Physics> physics);
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
    void AddDecor(std::string& object,
                  const glm::vec3 pos,
                  const glm::quat rot = glm::quat() );
    
    // Dynamic physics cubes testing
    DynaCube* AddDynaCube( const btVector3 & pos, const btVector3 & halfSize, const Color& col );
    void RemoveDynaCube( DynaCube* cube );

    // Objects
  //  Cubeject* LoadObject(const std::string& fileName );
  //  unsigned int AddObject(
		//const std::string& objectName,
		//const glm::vec3 position,
		//const glm::vec3 scale = glm::vec3(1.0f) );
  //  void RemoveObject( InstanceData* object );
  //  void RemoveObject( unsigned int objectID );
  //  Cubeject* GetObject(std::string& objectName );

    void Explosion( const glm::vec3 position, const float radius, const float force );
    
    
    static bool paused;                         // Used to switch off physics updates and freeze world
    static bool physicsEnabled;                 // Enable bullet physics engine
    static float worldTimeScale;                // 1.0 for normal speed, smaller for slo-mo
    
    std::string worldName;                      // Name of world, folder used for data streaming
    int seed;                                   // Random number which seeds cube generation
    int waterLevel;                             // Height of sea level in meters
    float waterWaveHeight;                      // Height of waves in meters
    
    int numContacts;                            // Number of physics contacts generated this frame
    //std::map<std::string, Cubeject*> cubejects; // Map of Cubejects sorted by their name
    
    LightInstance playerLight;                       // light at player pos
   
    bool refreshPhysics;                        // DEBUG: Do a full refresh on all physics vertices
    
    std::vector<glm::vec3> wantedTrees; // Positions for wanted trees
    
    // Entities
    int playerID;
    Coord3D playerCoord;    // TEMPORARY

private:
	std::shared_ptr<Injector> _injector;
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<Camera> _camera;
	std::shared_ptr<EntityManager> _entityMan;
	std::shared_ptr<Text> _text;
	std::shared_ptr<Options> _options;
	std::shared_ptr<Particles> _particles;
	std::shared_ptr<Physics> _physics;

    std::vector<DynaCube*> dynamicCubes;        // Dynamic cubes with physics
    std::vector<StaticCube*> staticCubes;       // Static cubes with physics
    std::vector<int> objectLabels;              // Labeling of nearby objects

	CubeInstance _floorCubes[32 * 32 * 32];
	int _numCubes;

	LightInstance _lights[4];
};


#endif
