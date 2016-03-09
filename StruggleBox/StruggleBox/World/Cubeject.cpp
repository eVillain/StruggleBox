//
//  Cubeject.cpp
//  Bloxelizer
//
//  Created by The Drudgerist on 7/23/13.
//
//

#include "Cubeject.h"
#include "SkyDome.h"
#include "Serialise.h"
#include "Renderer.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "SysCore.h"
#include <glm/gtc/matrix_transform.hpp>     // Matrix translate
#include <glm/gtx/rotate_vector.hpp>

int Cubeject::nextInstanceID = 1;
int Cubeject::totalCubejects = 0;

static inline int GetIndex(const int x, const int y, const int z, const int width_bits, const int height_bits ) {
    int index = x | (y | z << height_bits) << width_bits;
    return index;
};
static inline glm::vec3 GetPosForIndex(int index, const int width_bits, const int height_bits) {
    int z = index >> (width_bits + height_bits);
    int y = (index >> width_bits) & ((1 << height_bits) - 1);
    int x = index & ((1 << width_bits) - 1);
    return glm::vec3(x,y,z);
};

Cubeject::Cubeject( const std::string fileName, Renderer* renderer, const glm::vec3 pos, const int w_bits, const int h_bits ) {
    objectName = fileName;
    vBuffer = new VertexBuffer(-1, RType_NormalVerts);
    totalCubejects++;
    cubes = new CubeSet( w_bits, h_bits );
    renderSpheres = false;
//    AddInstance(pos);
}
Cubeject::Cubeject( const std::string fileName, Renderer* renderer ) {
    objectName = fileName;
    vBuffer = new VertexBuffer(-1, RType_NormalVerts);
    totalCubejects++;
    cubes = new CubeSet( fileName );
    renderSpheres = false;
}

Cubeject::~Cubeject() {
    if ( vBuffer != NULL ) {
        delete vBuffer;
        vBuffer = NULL;
    }
    if ( cubes ) {
        delete cubes;
        cubes = NULL;
    }
    totalCubejects--;
    instances.clear();
}
void Cubeject::Refresh() {
    cubes->changed = true;
}
void Cubeject::Update( const double delta ) {
    if ( cubes->changed ) {
        CacheMesh();
    }
    if ( renderSpheres && vBuffer->n_verts != NULL ) {
        CacheMesh();
    } else if ( !renderSpheres && vBuffer->c_verts != NULL ) {
        CacheMesh();
    }
}
void Cubeject::Draw( Renderer* renderer ) {
    if ( vBuffer != NULL && instances.size() > 0 && vBuffer->numVerts > 0 ) {
        renderer->RenderInstancedBuffer(vBuffer, vBuffer->instances, vBuffer->numVerts);
    }
}
void Cubeject::DrawTransparent( Renderer* renderer ) {
    if ( vBuffer != NULL && instances.size() > 0 && vBuffer->numTVerts > 0 ) {
        renderer->RenderInstancedBuffer(vBuffer, vBuffer->instances, vBuffer->numTVerts, vBuffer->numVerts);
    }
}
void Cubeject::CacheMesh() {
    if ( vBuffer == NULL ) return;
    
    int numCubes = cubes->GetNumCubes();
    unsigned int maxVerts = numCubes*36;
    vBuffer->numVerts = 0;
    vBuffer->numTVerts = 0;
    
    if ( renderSpheres ) {
        if ( vBuffer->c_verts == NULL ) {
            vBuffer->c_verts = new ColorVertexData[maxVerts];
        }
        if ( vBuffer->n_verts != NULL ) {
            delete [] vBuffer->n_verts;
            vBuffer->n_verts = NULL;
        }
        vBuffer->bufferType = RType_SphereVerts;
        cubes->GetMeshSpheres(vBuffer->c_verts, vBuffer->numVerts);
    } else {
        if ( vBuffer->n_verts == NULL ) {
            vBuffer->n_verts = new NormalVertexData[maxVerts];
        }
        if ( vBuffer->c_verts != NULL ) {
            delete [] vBuffer->c_verts;
            vBuffer->c_verts = NULL;
        }
		unsigned int numTVerts = 0;
		NormalVertexData*  t_verts = new NormalVertexData[maxVerts/2];

        vBuffer->bufferType = RType_NormalVerts;
        cubes->GetMeshLinear( vBuffer->n_verts, vBuffer->numVerts, t_verts, numTVerts );
//        cubes->GetMeshReduced( vBuffer->n_verts, vBuffer->numVerts, t_verts, numTVerts );
        if ( numTVerts > 0 ) {
            // Copy transparent verts to end of buffer
            if ( vBuffer->numVerts + numTVerts >= maxVerts ) {
                numTVerts = maxVerts - vBuffer->numVerts;
            }
            memcpy(&vBuffer->n_verts[vBuffer->numVerts], t_verts, numTVerts*sizeof(NormalVertexData));
            vBuffer->numTVerts = numTVerts;
        }
        delete [] t_verts;
        t_verts = NULL;
    }
    vBuffer->updated = true;
}

unsigned int Cubeject::AddInstance( const glm::vec3 pos, const glm::vec3 rot, const glm::vec3 scl ) {
    if (vBuffer == NULL ) {
        printf("Cubeject FAILAGE!\n");
        return -1;
    }
    vBuffer->instances.push_back({pos, glm::quat(rot), scl});
    InstanceData* newInstance = &vBuffer->instances.back();
    int instanceID = Cubeject::nextInstanceID++;
    instances[instanceID] = newInstance;
    if ( vBuffer->instances.size() > 1 ) {
        std::map<int, InstanceData*>::iterator it;
        int i=0;
        for (it = instances.begin(); it != instances.end(); it++) {
            if ( i == vBuffer->instances.size() ) break;
//            printf("Rewired pointer %p to %p\n", it->second, &(vBuffer->instances[i]));
            it->second = &(vBuffer->instances[i]);
            i++;
        }
    }
//    printf("[Cubeject] Added instance %i of %lu to %s\n", instanceID, vBuffer->instances.size(), objectName.c_str());
    return instanceID;
}
bool Cubeject::RemoveInstance( InstanceData* instance ) {
    std::map<int, InstanceData*>::iterator it;
    for ( it = instances.begin(); it != instances.end(); it++) {
        if ( instance == it->second ) {
            instances.erase(it);
            for (int i=0; i<vBuffer->instances.size(); i++) {
                if ( instance == &vBuffer->instances[i] ) {
                    vBuffer->instances.erase(vBuffer->instances.begin()+i);
                    // Need to rewire pointers to instance buffer
                    std::map<int, InstanceData*>::iterator it;
                    int j=0;
                    for (it = instances.begin(); it != instances.end(); it++) {
                        if ( j == vBuffer->instances.size() ) break;
                        it->second = &(vBuffer->instances[j]);
                        j++;
                    }
                    return true;
                }
            }
            printf("Instance removed from map but not vector!\n");
            return true;
        }
    }
    return false;
}
bool Cubeject::RemoveInstance( const int instanceID ) {
    if ( instances.find(instanceID) != instances.end() ) {
        InstanceData* instance = instances[instanceID];
        instances.erase(instances.find(instanceID));
        for (int i=0; i<vBuffer->instances.size(); i++) {
            if ( instance == &vBuffer->instances[i] ) {
                vBuffer->instances.erase(vBuffer->instances.begin()+i);
                // Need to rewire pointers to instance buffer
                std::map<int, InstanceData*>::iterator it;
                int j=0;
                for (it = instances.begin(); it != instances.end(); it++) {
                    if ( j == vBuffer->instances.size() ) break;
                    it->second = &(vBuffer->instances[j]);
                    j++;
                }
                return true;
            }
        }
        printf("Instance removed from map but not vector!\n");
        return true;
    }
    return false;
}
InstanceData* Cubeject::GetInstance( const int instanceID ) {
    if ( instances.find(instanceID) != instances.end() ) {
        return instances[instanceID];
    }
    printf("[Cubeject] no instance %i found in %s\n", instanceID, objectName.c_str());
    return NULL;
}
unsigned int Cubeject::GetInstanceID( const InstanceData* instance ) {
    std::map<int, InstanceData*>::iterator it;
    for ( it=instances.begin(); it != instances.end(); it++ ) {
        if ( it->second == instance ) {
            return it->first;
        }
    }
    return 0;
}

