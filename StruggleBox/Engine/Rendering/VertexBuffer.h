//
//  VertexBuffer.h
//  Ingenium
//
//  Created by The Drudgerist on 28/01/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//
//  This class provides an abstraction for graphical data
//  These may need to be managed by the renderer so we get
//  valid hardware IDs in case the current render context is
//  destroyed (eg. upon full-screen toggle)
//
//  The renderer should be able to draw a set of arbitrary
//  vertices from *verts or spheres from *s_verts
//  as well as repeating the object if *instances contains data
//  If no numInstances == 0 we just render the verts/spheres once
//
//  Transparent data follows the opaque data in *verts
//  ( Access it starting at verts[numVerts], ending at verts[numVerts+numTVerts] )
//
//  bufferType will define how the buffer should be treated
//  should be used by renderer to choose correct shader for rendering

#ifndef NGN_VERTEX_BUFFER_H
#define NGN_VERTEX_BUFFER_H

#include "GFXDefines.h"
#include <vector>

enum RenderType {
    RType_NormalVerts = 1,
    RType_ColorVerts = 2,
    RType_SpriteVerts = 3,
    RType_SphereVerts = 4,
    RType_CloudVerts = 5,
    RType_SkyVerts = 6,
};

class VertexBuffer {
    int hwID;          // Buffer hardware ID
public:
    VertexBuffer(int newID, RenderType type );
    ~VertexBuffer();
    const int GetID() { return hwID; };
    void SetID( const int newID ) { hwID = newID; };
    NormalVertexData* n_verts;    // Pointer to normal vertex data
    std::vector<InstanceData> instances;    // Pointer to instance data
    ColorVertexData* c_verts;        // Pointer to color data
    glm::vec4* p_verts;         // Simple 3D vertice position data
    unsigned int numVerts;      // Number of vertices in vertex data pointer
    unsigned int numTVerts;     // Number of transparent vertices in vertex data pointer
    bool updated;               // Vertices were updated since last buffer operation
    RenderType bufferType;      // How to render buffer
};

#endif /* defined(NGN_VERTEX_BUFFER_H) */

