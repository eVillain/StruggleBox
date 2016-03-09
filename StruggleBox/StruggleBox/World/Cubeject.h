//
//  Cubeject.h
//  Bloxelizer
//
//  Created by The Drudgerist on 7/23/13.
//
//

#ifndef BWO_CUBEJECT_H
#define BWO_CUBEJECT_H

#include "Block.h"
#include "Physics.h"
#include "CubeSet.h"
#include <glm/gtc/quaternion.hpp>
#include <map>
#include "VertexBuffer.h"


class Renderer;

class Cubeject {
private:
    // We hold reference pointers to instance data in the map, actual data is in vertexbuffer
    std::map<int, InstanceData*> instances;        // Map of IDs and instances
public:
    static int nextInstanceID;                              // ID for next created instance
    static int totalCubejects;                              // Total number of cubejects in memory
    std::string objectName;                                 // Unique name of object, will be used as filename
    CubeSet* cubes;                                         // Cube data set
    VertexBuffer* vBuffer;                                  // Vertex buffer object holding our drawable data
    bool renderSpheres;                                     // Render object as spheres instead of cubes

    Cubeject( const std::string fileName,                   // Create empty cubeject
              Renderer* renderer,
              const glm::vec3 pos,
              const int w_bits=DEFAULT_WIDTH_BITS,
              const int h_bits=DEFAULT_HEIGHT_BITS );
    Cubeject( const std::string fileName,                   // Create from file
              Renderer* renderer );
    ~Cubeject();
    
    // -- Rendering -- //
    void Refresh( void );
    void Update( const double delta );
    void Draw( Renderer* renderer );
    void DrawTransparent( Renderer* renderer );
    // -- Mesh building and caching -- //
    void CacheMesh( void );
    // -- Instancing -- //
    unsigned int AddInstance( const glm::vec3 pos, const glm::vec3 rot=glm::vec3(0), const glm::vec3 scl=glm::vec3(1) );
    bool RemoveInstance( InstanceData* instance );
    bool RemoveInstance( const int instanceID );
    InstanceData* GetInstance( const int instanceID );
    unsigned int GetInstanceID( const InstanceData* instance );
    unsigned long GetNumInstances( void ) { return instances.size(); };
};

#endif
