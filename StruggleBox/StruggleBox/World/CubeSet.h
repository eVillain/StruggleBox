//#ifndef CUBESET_H
//#define CUBESET_H
//
//// TODO: Rename this class to VoxelVolume
//#include "Block.h"
//#include "Physics.h"
//#include "AABB3D.h"
//#include <functional>
//
//#define DEFAULT_WIDTH_BITS 4
//#define DEFAULT_HEIGHT_BITS 4
//#define CUBESET_HEADER "BWOCB"  // Must be 5 characters
//#define CUBESET_VERSION "1.0"   // Must be 3 characters
//class World3D;
//class MaterialData;
//
//class CubeSet
//{
//public:
//    Block* blocks;          // Array of blocks
//    bool changed;           // Changed since last cache operation
//    CubeSet* neighborL;
//    CubeSet* neighborR;
//    CubeSet* neighborB;
//    CubeSet* neighborF;
//    CubeSet* neighborLB;
//    CubeSet* neighborRB;
//    CubeSet* neighborLF;
//    CubeSet* neighborRF;
//    
//    // -- Constructor & Destructor -- //
//    CubeSet( const int w_bits = DEFAULT_WIDTH_BITS,
//             const int h_bits = DEFAULT_HEIGHT_BITS );
//    CubeSet( const std::string fileName );
//    ~CubeSet();
//    // -- Size getters -- //
//    int GetWidth( void ) { return (1<<width_bits); };
//    int GetHeight( void ) { return (1<<height_bits); };
//    int GetNumCubes( void ) { return (1<<width_bits)*(1<<width_bits)*(1<<height_bits); };
//    // -- Object resizing -- //
//    void Resize( const int x_bits, const int y_bits );
//    void ShrinkHorizontal ( void );
//    void ShrinkVertical ( void );
//    void ExpandHorizontal ( void );
//    void ExpandVertical ( void );
//    // -- Block editing -- //
//    int PosToArray( const glm::vec3 pos );
//    Block& Get( const int x, const int y, const int z );
//    void Set( glm::vec3 pos, const Color& col, const BlockType type );
//    Block* GetNearestBlock( const glm::vec3& pos );
//    void Clear( void );
//    void Rotate( const bool ccw );
//    void MoveContents( const int moveX, const int moveY, const int moveZ );
//    void ReplaceColor( const Color& oldColor, const Color& newColor );
//    void ReplaceType( const BlockType oldType, const BlockType newType );
//    // -- Object generation -- //
//    void GenerateTree( const glm::vec3 treePos, const int seed );
//    void GenerateGrass( const int seed );
//    
////    void AddSphere( const glm::vec3 pos, const glm::vec3 radius, const BlockType type );
//    void SetVolume( const AABB3D volume, const BlockType type, const bool replace=false );
//    void SetVolume( const glm::vec3 pos, const glm::vec3 radius, const BlockType type, const bool replace=false );
//    void SetSphere( const glm::vec3 pos, const glm::vec3 radius, const BlockType type, const bool replace=false );
//    void SetCylinderY( const glm::vec3 pos, const glm::vec3 radius, const BlockType type, const bool replace=false );
//
//    // -- Mesh building and caching -- //
//    void GetMeshLinear(
//		MeshVertexData* o_verts,
//		unsigned int& numOVerts,
//        MeshVertexData* t_verts,
//		unsigned int& numTVerts,
//		MaterialData& materials,
//        glm::vec3 offset=glm::vec3());
//    //void GetMeshReduced( NormalVertexData* o_verts, unsigned int& numOVerts,
//    //                     NormalVertexData* t_verts, unsigned int& numTVerts,
//    //                     glm::vec3 offset=glm::vec3() );
//    void GetMeshSpheres( ColorVertexData* s_verts, unsigned int& numSVerts );
//    
//    void GetPhysicsLinear( btVector3* p_verts, unsigned int& numPVerts );
//    void GetPhysicsReduced( btVector3* p_verts, unsigned int& numPVerts, const float scale );
//    void GetPhysicsCubes( btCompoundShape* shape, const float scale );
//    void GetPhysicsHull( btConvexHullShape *shape, const float scale );
//    void GetPhysicsAABBs( btCompoundShape* shape, const float scale );
//    void GetDynaBlocks( World3D* world, const glm::vec3 position, const float scale );
//    
//    bool GetVisibility( const int x, const int y, const int z );
//    //bool GetVisibility( const int x, const int y, const int z, CubeSet* neighbor );
//    bool IsBlocked( const int x, const int y, const int z, const int type );
//    bool IsBlocked( const int x, const int y, const int z, const int type, std::function<class CubeSet*(int, int)> neighborFunc);
//    bool IsBlocked( const int x, const int y, const int z, const int type, CubeSet* neighbor );
//    bool PhysicsBlocked( int x, int y, int z );
//    bool ColorBlocked( int x, int y, int z, Color col );
//    // -- Data serialization -- //
//    int Serialise( unsigned char* buffer );
//    int Deserialise( const unsigned char* buffer );
//
//private:
//	int width_bits;         // Width/depth of object is two raised to the power of width_bits
//	int height_bits;        // Height of object is two raised to the power of height_bits
//};
//
//#endif
