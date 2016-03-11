#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(const int newID,
                           RenderType type) :
hwID(newID),
bufferType(type)
{
    n_verts = NULL;
    c_verts = NULL;
    p_verts = NULL;
    numVerts = 0;
    numTVerts = 0;
    updated = false;
}

VertexBuffer::~VertexBuffer()
{
    instances.clear();
    if ( n_verts != NULL ) {
        delete [] n_verts;
        n_verts = NULL;
    }
    if ( c_verts != NULL ) {
        delete [] c_verts;
        c_verts = NULL;
    }
    if ( p_verts != NULL ) {
        delete [] p_verts;
        p_verts = NULL;
    }
}
