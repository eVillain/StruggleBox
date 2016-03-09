#include <climits>
#include <fstream>
#include "CubeSet.h"
#include "FileUtil.h"
#include "Timer.h"
#include "Random.h"
#include "Serialise.h"
#include "Console.h"
#include "VoxelAABB.h"
#include "Timer.h"
#include <glm/gtc/noise.hpp> // glm::simplex
#include "World3D.h"

static inline int GetIndex(const int x, const int y, const int z,
                           const int width_bits, const int height_bits ) {
    int index = z | (y | x << height_bits) << width_bits;
    return index;
};
static inline glm::vec3 GetPosForIndex(int index, const int width_bits, const int height_bits) {
    int x = index >> (width_bits + height_bits);
    int y = (index >> width_bits) & ((1 << height_bits) - 1);
    int z = index & ((1 << width_bits) - 1);
    return glm::vec3(x,y,z);
};

CubeSet::CubeSet( const int w_bits, const int h_bits ) {
    width_bits = w_bits;
    height_bits = h_bits;
    
    int width = (1 << width_bits);
    int height = (1 << height_bits);
    int numBlocks = width*width*height;
    blocks = new Block[numBlocks];
    changed = false;
    
    neighborL=NULL; neighborR=NULL;
    neighborB=NULL; neighborF=NULL;
    neighborLB=NULL; neighborRB=NULL;
    neighborLF=NULL; neighborRF=NULL;
}
CubeSet::CubeSet( const std::string fileName ) {
    bool loaded = false;
    blocks = NULL;
    changed = false;
    neighborL=NULL; neighborR=NULL;
    neighborB=NULL; neighborF=NULL;
    neighborLB=NULL; neighborRB=NULL;
    neighborLF=NULL; neighborRF=NULL;
    
    if (fileName.length() > 0) {
        std::string folder =  FileUtil::GetPath().append("Data/Objects/");
        folder.append(fileName);
        std::ifstream::int_type size;
        unsigned char * data = NULL;
        //        std::ifstream file (fileName.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
        std::ifstream file (folder.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
        if ( file && file.is_open() ) {
            size = (int)file.tellg();
            data = new unsigned char[size];
            file.seekg (0, std::ios::beg);
            file.read ((char*)data, size);
            file.close();
            unsigned int dataPos = 0;
                // Pass data to object for deserialization
                dataPos += Deserialise(data+dataPos);
                loaded = true;
        } else Console::Print("Can't load file:%s\n", folder.c_str());
        delete[] data;
    } else Console::Print("Can't load file, no filename\n");
    if ( !loaded ) {
        Console::Print("Can't load file:%s\n", fileName.c_str());
        // Cancelled loading cube
        if ( !blocks ) {
            int width = (1 << width_bits);
            int height = (1 << height_bits);
            int numBlocks = width*width*height;
            blocks = new Block[numBlocks];
        }
    } else {
        printf("Loaded cubeset %s\n", fileName.c_str());
    }
}
CubeSet::~CubeSet() {
    delete [] blocks;
    blocks = NULL;
    changed = false;
}
void CubeSet::Resize(const int x_bits, const int y_bits) {
    if ( width_bits == x_bits && height_bits == y_bits ) return;
    if ( x_bits < 1 || y_bits < 1 ) return;
    // Get old size
    //    int widthOld = (1 << width_bits);
    //    int heightOld = (1 << height_bits);
    //    int numOldBlocks = widthOld*widthOld*heightOld;
    // Update size
    width_bits = x_bits;
    height_bits = y_bits;
    // Get new size
    int width = (1 << width_bits);
    int height = (1 << height_bits);
    int numBlocks = width*width*height;
    // Create new block storage
    Block* tempBlocks = new Block[numBlocks];
    // Copy old blocks over if possible
    if ( blocks ) {
        //        int numCopyBlocks = numOldBlocks > numBlocks ? numBlocks : numOldBlocks;
        //        memcpy(tempBlocks, blocks, sizeof(Block)*numCopyBlocks);
        // Clear old blocks
        delete [] blocks;
        blocks = NULL;
    }
    blocks = tempBlocks;
    changed = true;
}
void CubeSet::ShrinkHorizontal ( void ) {
    if (width_bits > 2) { Resize( width_bits-1, height_bits ); }
}
void CubeSet::ShrinkVertical ( void ) {
    if (height_bits > 2) { Resize( width_bits, height_bits-1 ); }
}
void CubeSet::ExpandHorizontal ( void ) {
    if (width_bits < 8) { Resize( width_bits+1, height_bits ); }
}
void CubeSet::ExpandVertical ( void ) {
    if (height_bits < 8) { Resize( width_bits, height_bits+1 ); }
}

inline int CubeSet::PosToArray( const glm::vec3 pos ) {
    float br2 = BLOCK_RADIUS*2.0f;
    float rW = BLOCK_RADIUS * GetWidth();
    float rH = BLOCK_RADIUS * GetHeight();
    glm::vec3 cPos = glm::vec3(rW, rH, rW);
    int arrX = (pos.x+cPos.x)/br2;
    int arrZ = (pos.z+cPos.z)/br2;
    int arrY = (pos.y+cPos.y)/br2;
    if (arrX < 0 || arrX > GetWidth()-1  ||
        arrY < 0 || arrY > GetHeight()-1  ||
        arrZ < 0 || arrZ > GetWidth()-1  ) { return -1; }
    int arrPos = GetIndex(arrX, arrY, arrZ, width_bits, height_bits);
    return arrPos;
}
inline Block& CubeSet::Get( int x, int y, int z ) {
    int arrPos = GetIndex(x, y, z, width_bits, height_bits);
    return blocks[arrPos];
}
void CubeSet::Set( glm::vec3 pos, const Color& col, const BlockType type ) {
    int maxCubes = GetWidth()*GetWidth()*GetHeight();
    int arrPos = PosToArray(pos);
    if ( arrPos < maxCubes && arrPos >= 0 ) {
        blocks[arrPos].blockType = type;
        blocks[arrPos].blockColor = col;
        changed = true;
    }
}
Block* CubeSet::GetNearestBlock( const glm::vec3& pos ) {
    int maxCubes = GetWidth()*GetWidth()*GetHeight();
    int arrPos = PosToArray(pos);
    if ( arrPos < maxCubes && arrPos >= 0 ) {
        return &blocks[arrPos];
    }
    return NULL;
}
void CubeSet::Clear( void ) {
    int width = (1 << width_bits);
    int height = (1 << height_bits);
    int numBlocks = width*width*height;
    for (int i=0; i<numBlocks; i++) {
        blocks[i].blockType = Type_Empty;
        blocks[i].blockColor = COLOR_NONE;
    }
    changed = true;
}
void CubeSet::Rotate( const bool ccw ) {
    if ( !blocks ) return;
    int width = (1 << width_bits);
    int height = (1 << height_bits);
    int numBlocks = width*width*height;
    Block* tempBlocks = new Block[numBlocks];
    for (int x=0; x<width; x++) {
        for (int z=0; z<width; z++) {
            for (int y=0; y<height; y++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                int tempX = ccw ? z : (width-1)-z;
                int tempZ = ccw ? (width-1)-x : x;
                int tempPos = GetIndex(tempX, y, tempZ, width_bits, height_bits);
                tempBlocks[tempPos] = blocks[arrPos];
            }
        }
    }
    delete [] blocks;
    blocks = tempBlocks;
    changed = true;
}
void CubeSet::MoveContents( const int moveX, const int moveY, const int moveZ ) {
    if ( moveX == 0 && moveY == 0 && moveZ == 0 ) return;
    if ( !blocks ) return;
    int moveOffsetX = GetIndex(moveX, 0, 0, width_bits, height_bits);
    int moveOffsetY = GetIndex(0, moveY, 0, width_bits, height_bits);
    int moveOffsetZ = GetIndex(0, 0, moveZ, width_bits, height_bits);
    int width = (1 << width_bits);
    int height = (1 << height_bits);
    int numBlocks = width*width*height;
    Block* tempBlocks = new Block[numBlocks];
    if ( moveOffsetX > 0 )  {
        memcpy(tempBlocks+(numBlocks-1)-moveOffsetX, blocks, sizeof(Block)*moveOffsetX);
        memcpy(tempBlocks, blocks+moveOffsetX, sizeof(Block)*(numBlocks-moveOffsetX));
    } else if ( moveOffsetX < 0 ) {
        moveOffsetX *= -1;
        memcpy(tempBlocks, blocks+(numBlocks-1)-moveOffsetX, sizeof(Block)*moveOffsetX);
        memcpy(tempBlocks+moveOffsetX, blocks, sizeof(Block)*(numBlocks-moveOffsetX));
    }
    if ( moveOffsetY > 0 )  {
        memcpy(tempBlocks+(numBlocks-1)-moveOffsetY, blocks, sizeof(Block)*moveOffsetY);
        memcpy(tempBlocks, blocks+moveOffsetY, sizeof(Block)*(numBlocks-moveOffsetY));
    } else if ( moveOffsetY < 0 ) {
        moveOffsetY *= -1;
        memcpy(tempBlocks, blocks+(numBlocks-1)-moveOffsetY, sizeof(Block)*moveOffsetY);
        memcpy(tempBlocks+moveOffsetY, blocks, sizeof(Block)*(numBlocks-moveOffsetY));
    }
    if ( moveOffsetZ > 0 )  {
        memcpy(tempBlocks+(numBlocks-1)-moveOffsetZ, blocks, sizeof(Block)*moveOffsetZ);
        memcpy(tempBlocks, blocks+moveOffsetZ, sizeof(Block)*(numBlocks-moveOffsetZ));
    } else if ( moveOffsetZ < 0 ) {
        moveOffsetZ *= -1;
        memcpy(tempBlocks, blocks+(numBlocks-1)-moveOffsetZ, sizeof(Block)*moveOffsetZ);
        memcpy(tempBlocks+moveOffsetZ, blocks, sizeof(Block)*(numBlocks-moveOffsetZ));
    }
    delete [] blocks;
    blocks = tempBlocks;
    changed = true;
}
void CubeSet::ReplaceColor( const Color& oldColor, const Color& newColor ) {
    int width = (1 << width_bits);
    int height = (1 << height_bits);
    int numBlocks = width*width*height;
    for (int i=0; i<numBlocks; i++) {
        if ( blocks[i].blockColor.r == oldColor.r &&
            blocks[i].blockColor.g == oldColor.g &&
            blocks[i].blockColor.b == oldColor.b &&
            blocks[i].blockColor.a == oldColor.a ) {
            blocks[i].blockColor = newColor;
        }
    }
    changed = true;
}
void CubeSet::ReplaceType( const BlockType oldType, const BlockType newType ) {
    int width = (1 << width_bits);
    int height = (1 << height_bits);
    int numBlocks = width*width*height;
    for (int i=0; i<numBlocks; i++) {
        if ( blocks[i].blockType == oldType ) {
            blocks[i].blockType = newType;
        }
    }
    changed = true;
}
void CubeSet::GetMeshLinear( NormalVertexData* o_verts, unsigned int& numOVerts,
                            NormalVertexData* t_verts, unsigned int& numTVerts,
                            glm::vec3 offset ) {
    int width = GetWidth();
    int height = GetHeight();
    int numCubes = GetNumCubes();

    float radius = BLOCK_RADIUS;
    float r2 = BLOCK_RADIUS * 2.0f;
    glm::vec3 blockOffset = glm::vec3(radius-(width*radius), radius-(height*radius), radius-(width*radius));

    int totalOVerts = 0;        // New Opaque verts
    int totalTVerts = 0;        // New Transparent verts
    
    for (int i=0; i<numCubes; i++) {
        Block& b = blocks[i];
        if ( b.blockType > Type_Empty ) {
            int x = i >> (width_bits + height_bits);
            int y = (i >> width_bits) & ((1 << height_bits) - 1);
            int z = i & ((1 << width_bits) - 1);
            
            Color diff = b.blockColor;
            float spec = SpecularForType(b.blockType);

            int cubeVerts = 0;  // Verts for this block
            NormalVertexData* gBuffer = NULL;
            if ( diff.a == 1.0f ) { gBuffer = o_verts+totalOVerts; }
            else { gBuffer = t_verts+totalTVerts; }
            

            glm::vec3 pos = (glm::vec3(x,y,z)*r2)+offset+blockOffset;

            // Add triangles to mesh
//            bool lVis = !IsBlocked(x-1, y, z, b.blockType);
//            bool rVis = !IsBlocked(x+1, y, z, b.blockType);
//            bool bVis = !IsBlocked(x, y-1, z, b.blockType);
//            bool tVis = !IsBlocked(x, y+1, z, b.blockType);
//            bool reVis = !IsBlocked(x, y, z-1, b.blockType);
//            bool frVis = !IsBlocked(x, y, z+1, b.blockType);
            bool lVis = !PhysicsBlocked(x-1, y, z);
            bool rVis = !PhysicsBlocked(x+1, y, z);
            bool bVis = !PhysicsBlocked(x, y-1, z);
            bool tVis = !PhysicsBlocked(x, y+1, z);
            bool reVis = !PhysicsBlocked(x, y, z-1);
            bool frVis = !PhysicsBlocked(x, y, z+1);

            if ( lVis || rVis || bVis || tVis || reVis || frVis ) {
                GLfloat occlusion = 0.25f;
                // Each vertex has 4 neighbors to be checked for ambient occlusion
                // These are the amounts for each neighbor node
                GLfloat lbao = ( GetVisibility(x-1, y-1, z ) ) ? occlusion : 0.0f;
                GLfloat ltao = ( GetVisibility(x-1, y+1, z ) ) ? occlusion : 0.0f;
                GLfloat rbao = ( GetVisibility(x+1, y-1, z ) ) ? occlusion : 0.0f;
                GLfloat rtao = ( GetVisibility(x+1, y+1, z ) ) ? occlusion : 0.0f;
                GLfloat brao = ( GetVisibility(x, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat trao = ( GetVisibility(x, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat bfao = ( GetVisibility(x, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat tfao = ( GetVisibility(x, y+1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat lrao = ( GetVisibility(x-1, y, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rrao = ( GetVisibility(x+1, y, z-1 ) ) ? occlusion : 0.0f;
                GLfloat lfao = ( GetVisibility(x-1, y, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rfao = ( GetVisibility(x+1, y, z+1 ) ) ? occlusion : 0.0f;
                GLfloat lbrao = ( GetVisibility(x-1, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat lbfao = ( GetVisibility(x-1, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat ltrao = ( GetVisibility(x-1, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat ltfao = ( GetVisibility(x-1, y+1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rbrao = ( GetVisibility(x+1, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rbfao = ( GetVisibility(x+1, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rtrao = ( GetVisibility(x+1, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rtfao = ( GetVisibility(x+1, y+1, z+1 ) ) ? occlusion : 0.0f;

                if ( lVis ) {
                    NormalVertexData lbr = { pos.x-radius, pos.y-radius, pos.z-radius, occlusion+lbao+lrao+lbrao,
                        diff.r,diff.g,diff.b,diff.a,spec, -1.0f,0.0f,0.0f };
                    NormalVertexData lbf = { pos.x-radius, pos.y-radius, pos.z+radius, occlusion+lbao+lfao+lbfao,
                        diff.r,diff.g,diff.b,diff.a,spec, -1.0f,0.0f,0.0f };
                    NormalVertexData ltr = { pos.x-radius, pos.y+radius, pos.z-radius, occlusion+ltao+lrao+ltrao,
                        diff.r,diff.g,diff.b,diff.a,spec, -1.0f,0.0f,0.0f };
                    NormalVertexData ltf = { pos.x-radius, pos.y+radius, pos.z+radius, occlusion+ltao+lfao+ltfao,
                        diff.r,diff.g,diff.b,diff.a,spec, -1.0f,0.0f,0.0f };
                    gBuffer[cubeVerts++] = ltf;
                    gBuffer[cubeVerts++] = ltr;
                    gBuffer[cubeVerts++] = lbr;
                    gBuffer[cubeVerts++] = lbr;
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = ltf;
                }

                if ( rVis ) {
                    NormalVertexData rbr = { pos.x+radius, pos.y-radius, pos.z-radius, occlusion+rbao+rrao+rbrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 1.0f,0.0f,0.0f };
                    NormalVertexData rbf = { pos.x+radius, pos.y-radius, pos.z+radius, occlusion+rbao+rfao+rbfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 1.0f,0.0f,0.0f };
                    NormalVertexData rtr = { pos.x+radius, pos.y+radius, pos.z-radius, occlusion+rtao+rrao+rtrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 1.0f,0.0f,0.0f };
                    NormalVertexData rtf = { pos.x+radius, pos.y+radius, pos.z+radius, occlusion+rtao+rfao+rtfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 1.0f,0.0f,0.0f };
                    gBuffer[cubeVerts++] = rtr;
                    gBuffer[cubeVerts++] = rtf;
                    gBuffer[cubeVerts++] = rbf;
                    gBuffer[cubeVerts++] = rbf;
                    gBuffer[cubeVerts++] = rbr;
                    gBuffer[cubeVerts++] = rtr;
                }
                if ( bVis ) {
                    NormalVertexData lbr = { pos.x-radius, pos.y-radius, pos.z-radius, occlusion+brao+brao+lbrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,-1.0f,0.0f };
                    NormalVertexData lbf = { pos.x-radius, pos.y-radius, pos.z+radius, occlusion+bfao+bfao+lbfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,-1.0f,0.0f };
                    NormalVertexData rbr = { pos.x+radius, pos.y-radius, pos.z-radius, occlusion+brao+brao+rbrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,-1.0f,0.0f };
                    NormalVertexData rbf = { pos.x+radius, pos.y-radius, pos.z+radius, occlusion+bfao+bfao+rbfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,-1.0f,0.0f };
                    gBuffer[cubeVerts++] = rbr;
                    gBuffer[cubeVerts++] = rbf;
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = lbr;
                    gBuffer[cubeVerts++] = rbr;
                }
                if ( tVis ) {
                    NormalVertexData ltr = { pos.x-radius, pos.y+radius, pos.z-radius, occlusion+ltao+trao+ltrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,1.0f,0.0f };
                    NormalVertexData ltf = { pos.x-radius, pos.y+radius, pos.z+radius, occlusion+ltao+tfao+ltfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,1.0f,0.0f };
                    NormalVertexData rtr = { pos.x+radius, pos.y+radius, pos.z-radius, occlusion+rtao+trao+rtrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,1.0f,0.0f };
                    NormalVertexData rtf = { pos.x+radius, pos.y+radius, pos.z+radius, occlusion+rtao+tfao+rtfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,1.0f,0.0f };
                    gBuffer[cubeVerts++] = ltr;
                    gBuffer[cubeVerts++] = ltf;
                    gBuffer[cubeVerts++] = rtf;
                    gBuffer[cubeVerts++] = rtf;
                    gBuffer[cubeVerts++] = rtr;
                    gBuffer[cubeVerts++] = ltr;
                }
                if ( reVis ) {
                    NormalVertexData lbr = { pos.x-radius, pos.y-radius, pos.z-radius, occlusion+brao+lrao+lbrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,-1.0f };
                    NormalVertexData ltr = { pos.x-radius, pos.y+radius, pos.z-radius, occlusion+trao+lrao+ltrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,-1.0f };
                    NormalVertexData rbr = { pos.x+radius, pos.y-radius, pos.z-radius, occlusion+brao+rrao+rbrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,-1.0f };
                    NormalVertexData rtr = { pos.x+radius, pos.y+radius, pos.z-radius, occlusion+trao+rrao+rtrao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,-1.0f };
                    gBuffer[cubeVerts++] = ltr;
                    gBuffer[cubeVerts++] = rtr;
                    gBuffer[cubeVerts++] = rbr;
                    gBuffer[cubeVerts++] = rbr;
                    gBuffer[cubeVerts++] = lbr;
                    gBuffer[cubeVerts++] = ltr;
                }
                if ( frVis ) {
                    NormalVertexData lbf = { pos.x-radius, pos.y-radius, pos.z+radius, occlusion+bfao+lfao+lbfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,1.0f };
                    NormalVertexData ltf = { pos.x-radius, pos.y+radius, pos.z+radius, occlusion+tfao+lfao+ltfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,1.0f };
                    NormalVertexData rbf = { pos.x+radius, pos.y-radius, pos.z+radius, occlusion+bfao+rfao+rbfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,1.0f };
                    NormalVertexData rtf = { pos.x+radius, pos.y+radius, pos.z+radius, occlusion+tfao+rfao+rtfao,
                        diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,1.0f };
                    gBuffer[cubeVerts++] = rtf;
                    gBuffer[cubeVerts++] = ltf;
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = rbf;
                    gBuffer[cubeVerts++] = rtf;
                }
                if ( diff.a == 1.0f ) { totalOVerts += cubeVerts; }
                else { totalTVerts += cubeVerts; }
            }
        }
    }
    numOVerts += totalOVerts;
    numTVerts += totalTVerts;
    changed = false;
}

void CubeSet::GetMeshReduced( NormalVertexData* o_verts, unsigned int& numOVerts,
                              NormalVertexData* t_verts, unsigned int& numTVerts,
                              glm::vec3 offset ) {    
    int width = GetWidth();
    int height = GetHeight();
    
    int merged = 0;
    bool vis = false;;
    int totalOVerts = 0;
    int totalTVerts = 0;
    
    float radius = BLOCK_RADIUS;
    float r2 = BLOCK_RADIUS * 2.0f;
    glm::vec3 blockOffset = offset + glm::vec3(radius-(width*radius), radius-(height*radius), radius-(width*radius));
    
    // View from negative x
    for(int x = width - 1; x >= 0; x--) {
        for(int y = 0; y < height; y++) {
            for(int z = 0; z < width; z++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                Color diff = b.blockColor;
                float spec = SpecularForType(b.blockType);
                // Empty or line of sight blocked?
                if ( b.blockType == Type_Empty ||
                    PhysicsBlocked(x-1, y, z) ) {
                    vis = false;
                    continue;
                }
                int cubeVerts = 0;
                NormalVertexData* gBuffer = NULL;
                if ( diff.a == 1.0f ) { gBuffer = o_verts+totalOVerts; }
                else { gBuffer = t_verts+totalTVerts; }
                
                GLfloat occlusion = 0.25f;
                // Each vertex has 4 neighbors to be checked for ambient occlusion
                // These are the amounts for each neighbor node
                GLfloat lbao = ( GetVisibility(x-1, y-1, z ) ) ? occlusion : 0.0f;
                GLfloat ltao = ( GetVisibility(x-1, y+1, z ) ) ? occlusion : 0.0f;
                GLfloat lrao = ( GetVisibility(x-1, y, z-1 ) ) ? occlusion : 0.0f;
                GLfloat lfao = ( GetVisibility(x-1, y, z+1 ) ) ? occlusion : 0.0f;
                GLfloat lbrao = ( GetVisibility(x-1, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat lbfao = ( GetVisibility(x-1, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat ltrao = ( GetVisibility(x-1, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat ltfao = ( GetVisibility(x-1, y+1, z+1 ) ) ? occlusion : 0.0f;
                
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                NormalVertexData lbr = { pos.x-radius, pos.y-radius, pos.z-radius, occlusion+lbao+lrao+lbrao,
                    diff.r,diff.g,diff.b,diff.a,spec, -1.0f,0.0f,0.0f };
                NormalVertexData lbf = { pos.x-radius, pos.y-radius, pos.z+radius, occlusion+lbao+lfao+lbfao,
                    diff.r,diff.g,diff.b,diff.a,spec, -1.0f,0.0f,0.0f };
                NormalVertexData ltr = { pos.x-radius, pos.y+radius, pos.z-radius, occlusion+ltao+lrao+ltrao,
                    diff.r,diff.g,diff.b,diff.a,spec, -1.0f,0.0f,0.0f };
                NormalVertexData ltf = { pos.x-radius, pos.y+radius, pos.z+radius, occlusion+ltao+lfao+ltfao,
                    diff.r,diff.g,diff.b,diff.a,spec, -1.0f,0.0f,0.0f };
                
                bool potentialMerge = false;
                if ( vis && z != 0 ) {  // Same block as previous one? Extend it.
                    Block& b2 = blocks[GetIndex(x, y, z-1, width_bits, height_bits)];
                    if ( b.blockColor == b2.blockColor &&
                        gBuffer[cubeVerts - 6].w == lbr.w &&
                        gBuffer[cubeVerts - 5].w == lbf.w &&
                        gBuffer[cubeVerts - 4].w == ltr.w &&
                        gBuffer[cubeVerts - 1].w == ltf.w ) {
                        potentialMerge = true;
                    }
                }
                if( potentialMerge ) {
                    gBuffer[cubeVerts - 5] = lbf;
                    gBuffer[cubeVerts - 2] = lbf;
                    gBuffer[cubeVerts - 1] = ltf;
                    merged++;
                } else {                                // Otherwise, add a new quad.
                    gBuffer[cubeVerts++] = lbr;
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = ltr;
                    gBuffer[cubeVerts++] = ltr;
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = ltf;
                }
                vis = true;
                if ( diff.a == 1.0f ) { totalOVerts += cubeVerts; }
                else { totalTVerts += cubeVerts; }
            }
        }
    }
    vis = false;

    // View from positive x
    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            for(int z = 0; z < width; z++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                Color diff = b.blockColor;
                float spec = SpecularForType(b.blockType);
                if ( b.blockType == Type_Empty ||
                    PhysicsBlocked(x+1, y, z) ) {
                    vis = false;
                    continue;
                }
                int cubeVerts = 0;
                NormalVertexData* gBuffer = NULL;
                if ( diff.a == 1.0f ) { gBuffer = o_verts+totalOVerts; }
                else { gBuffer = t_verts+totalTVerts; }
                
                GLfloat occlusion = 0.25f;
                // Each vertex has 4 neighbors to be checked for ambient occlusion
                // These are the amounts for each neighbor node
                GLfloat rbao = ( GetVisibility(x+1, y-1, z ) ) ? occlusion : 0.0f;
                GLfloat rtao = ( GetVisibility(x+1, y+1, z ) ) ? occlusion : 0.0f;
                GLfloat rrao = ( GetVisibility(x+1, y, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rfao = ( GetVisibility(x+1, y, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rbrao = ( GetVisibility(x+1, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rbfao = ( GetVisibility(x+1, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rtrao = ( GetVisibility(x+1, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rtfao = ( GetVisibility(x+1, y+1, z+1 ) ) ? occlusion : 0.0f;
                
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                NormalVertexData rbr = { pos.x+radius, pos.y-radius, pos.z-radius, occlusion+rbao+rrao+rbrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 1.0f,0.0f,0.0f };
                NormalVertexData rbf = { pos.x+radius, pos.y-radius, pos.z+radius, occlusion+rbao+rfao+rbfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 1.0f,0.0f,0.0f };
                NormalVertexData rtr = { pos.x+radius, pos.y+radius, pos.z-radius, occlusion+rtao+rrao+rtrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 1.0f,0.0f,0.0f };
                NormalVertexData rtf = { pos.x+radius, pos.y+radius, pos.z+radius, occlusion+rtao+rfao+rtfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 1.0f,0.0f,0.0f };
                
                bool potentialMerge = false;
                if ( vis && z != 0 ) {  // Same block as previous one? Extend it.
                    Block& b2 = blocks[GetIndex(x, y, z-1, width_bits, height_bits)];
                    if ( b.blockColor == b2.blockColor &&
                         gBuffer[cubeVerts - 6].w == rbr.w &&
                         gBuffer[cubeVerts - 5].w == rtr.w &&
                         gBuffer[cubeVerts - 2].w == rtf.w &&
                         gBuffer[cubeVerts - 1].w == rbf.w ) {
                        potentialMerge = true;
                    }
                }
                if( potentialMerge ) {
                    gBuffer[cubeVerts - 4] = rbf;
                    gBuffer[cubeVerts - 2] = rtf;
                    gBuffer[cubeVerts - 1] = rbf;
                    merged++;
                } else {
                    gBuffer[cubeVerts++] = rbr;
                    gBuffer[cubeVerts++] = rtr;
                    gBuffer[cubeVerts++] = rbf;
                    gBuffer[cubeVerts++] = rtr;
                    gBuffer[cubeVerts++] = rtf;
                    gBuffer[cubeVerts++] = rbf;
                }
                vis = true;
                if ( diff.a == 1.0f ) { totalOVerts += cubeVerts; }
                else { totalTVerts += cubeVerts; }
            }
        }
    }
    vis = false;

    // View from negative y
    for(int x = 0; x < width; x++) {
        for(int y = height - 1; y >= 0; y--) {
            for(int z = 0; z < width; z++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                Color diff = b.blockColor;
                float spec = SpecularForType(b.blockType);
                if ( b.blockType == Type_Empty ||
                    PhysicsBlocked(x, y-1, z) ) {
                    vis = false;
                    continue;
                }
                int cubeVerts = 0;
                NormalVertexData* gBuffer = NULL;
                if ( diff.a == 1.0f ) { gBuffer = o_verts+totalOVerts; }
                else { gBuffer = t_verts+totalTVerts; }
                
                GLfloat occlusion = 0.25f;
                // Each vertex has 4 neighbors to be checked for ambient occlusion
                // These are the amounts for each neighbor node
                GLfloat brao = ( GetVisibility(x, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat bfao = ( GetVisibility(x, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat lbrao = ( GetVisibility(x-1, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat lbfao = ( GetVisibility(x-1, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rbrao = ( GetVisibility(x+1, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rbfao = ( GetVisibility(x+1, y-1, z+1 ) ) ? occlusion : 0.0f;
                
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                NormalVertexData lbr = { pos.x-radius, pos.y-radius, pos.z-radius, occlusion+brao+brao+lbrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,-1.0f,0.0f };
                NormalVertexData lbf = { pos.x-radius, pos.y-radius, pos.z+radius, occlusion+bfao+bfao+lbfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,-1.0f,0.0f };
                NormalVertexData rbr = { pos.x+radius, pos.y-radius, pos.z-radius, occlusion+brao+brao+rbrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,-1.0f,0.0f };
                NormalVertexData rbf = { pos.x+radius, pos.y-radius, pos.z+radius, occlusion+bfao+bfao+rbfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,-1.0f,0.0f };
                
                bool potentialMerge = false;
                if ( vis && z != 0 ) {  // Same block as previous one? Extend it.
                    Block& b2 = blocks[GetIndex(x, y, z-1, width_bits, height_bits)];
                    if ( b.blockColor == b2.blockColor &&
                        gBuffer[cubeVerts - 6].w == lbr.w &&
                        gBuffer[cubeVerts - 5].w == rbr.w &&
                        gBuffer[cubeVerts - 2].w == rbf.w &&
                        gBuffer[cubeVerts - 1].w == lbf.w ) {
                        potentialMerge = true;
                    }
                }
                if( potentialMerge ) {
                    gBuffer[cubeVerts - 4] = lbf;
                    gBuffer[cubeVerts - 2] = rbf;
                    gBuffer[cubeVerts - 1] = lbf;
                    merged++;
                } else {
                    gBuffer[cubeVerts++] = lbr;
                    gBuffer[cubeVerts++] = rbr;
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = rbr;
                    gBuffer[cubeVerts++] = rbf;
                    gBuffer[cubeVerts++] = lbf;
                }
                vis = true;
                if ( diff.a == 1.0f ) { totalOVerts += cubeVerts; }
                else { totalTVerts += cubeVerts; }
            }
        }
    }
    vis = false;

    // View from positive y
    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            for(int z = 0; z < width; z++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                Color diff = b.blockColor;
                float spec = SpecularForType(b.blockType);
                // Empty or line of sight blocked?
                if ( b.blockType == Type_Empty ||
                    PhysicsBlocked(x, y+1, z) ) {
                    vis = false;
                    continue;
                }
                int cubeVerts = 0;
                NormalVertexData* gBuffer = NULL;
                if ( diff.a == 1.0f ) { gBuffer = o_verts+totalOVerts; }
                else { gBuffer = t_verts+totalTVerts; }
                
                GLfloat occlusion = 0.25f;
                // Each vertex has 4 neighbors to be checked for ambient occlusion
                // These are the amounts for each neighbor node
                GLfloat ltao = ( GetVisibility(x-1, y+1, z ) ) ? occlusion : 0.0f;
                GLfloat rtao = ( GetVisibility(x+1, y+1, z ) ) ? occlusion : 0.0f;
                GLfloat trao = ( GetVisibility(x, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat tfao = ( GetVisibility(x, y+1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat ltrao = ( GetVisibility(x-1, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat ltfao = ( GetVisibility(x-1, y+1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rtrao = ( GetVisibility(x+1, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rtfao = ( GetVisibility(x+1, y+1, z+1 ) ) ? occlusion : 0.0f;
                
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                NormalVertexData ltr = { pos.x-radius, pos.y+radius, pos.z-radius, occlusion+ltao+trao+ltrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,1.0f,0.0f };
                NormalVertexData ltf = { pos.x-radius, pos.y+radius, pos.z+radius, occlusion+ltao+tfao+ltfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,1.0f,0.0f };
                NormalVertexData rtr = { pos.x+radius, pos.y+radius, pos.z-radius, occlusion+rtao+trao+rtrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,1.0f,0.0f };
                NormalVertexData rtf = { pos.x+radius, pos.y+radius, pos.z+radius, occlusion+rtao+tfao+rtfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,1.0f,0.0f };
                
                bool potentialMerge = false;
                if ( vis && z != 0 ) {  // Same block as previous one? Extend it.
                    Block& b2 = blocks[GetIndex(x, y, z-1, width_bits, height_bits)];
                    if ( b.blockColor == b2.blockColor &&
                         gBuffer[cubeVerts - 6].w == ltr.w &&
                         gBuffer[cubeVerts - 4].w == rtr.w &&
                         gBuffer[cubeVerts - 2].w == ltf.w &&
                         gBuffer[cubeVerts - 1].w == rtf.w) {
                        potentialMerge = true;
                    }
                }
                if( potentialMerge ) {
                    gBuffer[cubeVerts - 5] = ltf;
                    gBuffer[cubeVerts - 2] = ltf;
                    gBuffer[cubeVerts - 1] = rtf;
                    merged++;
                } else {
                    gBuffer[cubeVerts++] = ltr;
                    gBuffer[cubeVerts++] = ltf;
                    gBuffer[cubeVerts++] = rtr;
                    gBuffer[cubeVerts++] = rtr;
                    gBuffer[cubeVerts++] = ltf;
                    gBuffer[cubeVerts++] = rtf;
                }
                vis = true;
                if ( diff.a == 1.0f ) { totalOVerts += cubeVerts; }
                else { totalTVerts += cubeVerts; }
            }
        }
    }
    vis = false;

    // View from negative z
    for(int x = 0; x < width; x++) {
        for(int z = width - 1; z >= 0; z--) {
            for(int y = 0; y < height; y++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                Color diff = b.blockColor;
                float spec = SpecularForType(b.blockType);
                // Empty or line of sight blocked?
                if ( b.blockType == Type_Empty ||
                    PhysicsBlocked(x, y, z-1) ) {
                    vis = false;
                    continue;
                }
                int cubeVerts = 0;
                NormalVertexData* gBuffer = NULL;
                if ( diff.a == 1.0f ) { gBuffer = o_verts+totalOVerts; }
                else { gBuffer = t_verts+totalTVerts; }
                
                GLfloat occlusion = 0.25f;
                // Each vertex has 4 neighbors to be checked for ambient occlusion
                // These are the amounts for each neighbor node
                GLfloat brao = ( GetVisibility(x, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat trao = ( GetVisibility(x, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat lrao = ( GetVisibility(x-1, y, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rrao = ( GetVisibility(x+1, y, z-1 ) ) ? occlusion : 0.0f;
                GLfloat lbrao = ( GetVisibility(x-1, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat ltrao = ( GetVisibility(x-1, y+1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rbrao = ( GetVisibility(x+1, y-1, z-1 ) ) ? occlusion : 0.0f;
                GLfloat rtrao = ( GetVisibility(x+1, y+1, z-1 ) ) ? occlusion : 0.0f;

                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                NormalVertexData lbr = { pos.x-radius, pos.y-radius, pos.z-radius, occlusion+brao+lrao+lbrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,-1.0f };
                NormalVertexData ltr = { pos.x-radius, pos.y+radius, pos.z-radius, occlusion+trao+lrao+ltrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,-1.0f };
                NormalVertexData rbr = { pos.x+radius, pos.y-radius, pos.z-radius, occlusion+brao+rrao+rbrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,-1.0f };
                NormalVertexData rtr = { pos.x+radius, pos.y+radius, pos.z-radius, occlusion+trao+rrao+rtrao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,-1.0f };
                
                bool potentialMerge = false;
                if ( vis && y != 0 ) {  // Same block as previous one? Extend it.
                    Block& b2 = blocks[GetIndex(x, y-1, z, width_bits, height_bits)];
                    if ( b.blockColor == b2.blockColor &&
                        gBuffer[cubeVerts - 6].w == lbr.w &&
                        gBuffer[cubeVerts - 5].w == ltr.w &&
                        gBuffer[cubeVerts - 2].w == rtr.w &&
                        gBuffer[cubeVerts - 1].w == rbr.w ) {
                        potentialMerge = true;
                    }
                }
                if( potentialMerge ) {
                    gBuffer[cubeVerts - 5] = ltr;
                    gBuffer[cubeVerts - 3] = ltr;
                    gBuffer[cubeVerts - 2] = rtr;
                    merged++;
                } else {
                    gBuffer[cubeVerts++] = lbr;
                    gBuffer[cubeVerts++] = ltr;
                    gBuffer[cubeVerts++] = rbr;
                    gBuffer[cubeVerts++] = ltr;
                    gBuffer[cubeVerts++] = rtr;
                    gBuffer[cubeVerts++] = rbr;
                }
                vis = true;
                if ( diff.a == 1.0f ) { totalOVerts += cubeVerts; }
                else { totalTVerts += cubeVerts; }
            }
        }
    }
    vis = false;
    // View from positive z
    for(int x = 0; x < width; x++) {
        for(int z = 0; z < width; z++) {
            for(int y = 0; y < height; y++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                Color diff = b.blockColor;
                float spec = SpecularForType(b.blockType);
                // Empty or line of sight blocked?
                if ( b.blockType == Type_Empty ||
                    PhysicsBlocked(x, y, z+1) ) {
                    vis = false;
                    continue;
                }
                int cubeVerts = 0;
                NormalVertexData* gBuffer = NULL;
                if ( diff.a == 1.0f ) { gBuffer = o_verts+totalOVerts; }
                else { gBuffer = t_verts+totalTVerts; }
                
                GLfloat occlusion = 0.25f;
                // Each vertex has 4 neighbors to be checked for ambient occlusion
                // These are the amounts for each neighbor node
                GLfloat bfao = ( GetVisibility(x, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat tfao = ( GetVisibility(x, y+1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat lfao = ( GetVisibility(x-1, y, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rfao = ( GetVisibility(x+1, y, z+1 ) ) ? occlusion : 0.0f;
                GLfloat lbfao = ( GetVisibility(x-1, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat ltfao = ( GetVisibility(x-1, y+1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rbfao = ( GetVisibility(x+1, y-1, z+1 ) ) ? occlusion : 0.0f;
                GLfloat rtfao = ( GetVisibility(x+1, y+1, z+1 ) ) ? occlusion : 0.0f;
                
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                NormalVertexData lbf = { pos.x-radius, pos.y-radius, pos.z+radius, occlusion+bfao+lfao+lbfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,1.0f };
                NormalVertexData ltf = { pos.x-radius, pos.y+radius, pos.z+radius, occlusion+tfao+lfao+ltfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,1.0f };
                NormalVertexData rbf = { pos.x+radius, pos.y-radius, pos.z+radius, occlusion+bfao+rfao+rbfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,1.0f };
                NormalVertexData rtf = { pos.x+radius, pos.y+radius, pos.z+radius, occlusion+tfao+rfao+rtfao,
                    diff.r,diff.g,diff.b,diff.a,spec, 0.0f,0.0f,1.0f };
                
                bool potentialMerge = false;
                if ( vis && y != 0 ) {  // Same block as previous one? Extend it.
                    Block& b2 = blocks[GetIndex(x, y-1, z, width_bits, height_bits)];
                    if ( b.blockColor == b2.blockColor &&
                        gBuffer[cubeVerts - 6].w == lbf.w &&
                        gBuffer[cubeVerts - 5].w == rbf.w &&
                        gBuffer[cubeVerts - 4].w == ltf.w &&
                        gBuffer[cubeVerts - 1].w == rtf.w ) {
                        potentialMerge = true;
                    }
                }
                if( potentialMerge ) {
                    gBuffer[cubeVerts - 4] = ltf;
                    gBuffer[cubeVerts - 3] = ltf;
                    gBuffer[cubeVerts - 1] = rtf;
                    merged++;
                } else {
                    gBuffer[cubeVerts++] = lbf;
                    gBuffer[cubeVerts++] = rbf;
                    gBuffer[cubeVerts++] = ltf;
                    gBuffer[cubeVerts++] = ltf;
                    gBuffer[cubeVerts++] = rbf;
                    gBuffer[cubeVerts++] = rtf;
                }
                vis = true;
                if ( diff.a == 1.0f ) { totalOVerts += cubeVerts; }
                else { totalTVerts += cubeVerts; }
            }
        }
    }
    
    numOVerts += totalOVerts;
    numTVerts += totalTVerts;
}
void CubeSet::GetMeshSpheres(ColorVertexData *s_verts, unsigned int &numSVerts) {
    
    int width = GetWidth();
    int height = GetHeight();
    int numCubes = GetNumCubes();
    
    float radius = BLOCK_RADIUS;
    float r2 = BLOCK_RADIUS * 2.0f;
    
    // Make verts relative to position
    //    glm::vec3 cPos = position;
    
    int totalSVerts = 0;        // Sphere verts
    
    for (int i=0; i<numCubes; i++) {
        Block& b = blocks[i];
        if ( b.blockType > Type_Empty ) {
            int x = i >> (width_bits + height_bits);
            int y = (i >> width_bits) & ((1 << height_bits) - 1);
            int z = i & ((1 << width_bits) - 1);
            Color col = b.blockColor;
            
            glm::vec3 offset = glm::vec3(radius-(width*radius), radius-(height*radius), radius-(width*radius));
//            glm::vec3 pos = cPos+glm::vec3(x*r2,y*r2,z*r2)+offset;
            glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+offset;
            
            // Add triangles to mesh
            bool lVis = !IsBlocked(x-1, y, z, b.blockType);
            bool rVis = !IsBlocked(x+1, y, z, b.blockType);
            bool bVis = !IsBlocked(x, y-1, z, b.blockType);
            bool tVis = !IsBlocked(x, y+1, z, b.blockType);
            bool reVis = !IsBlocked(x, y, z-1, b.blockType);
            bool fVis = !IsBlocked(x, y, z+1, b.blockType);
            if ( lVis || rVis || bVis || tVis || reVis || fVis ) {
                    ColorVertexData sphere = {pos.x, pos.y, pos.z, radius, col.r, col.g, col.b, col.a};
                    s_verts[totalSVerts++] = sphere;
            }
        }
    }
    numSVerts += totalSVerts;
    changed = false;
}
void CubeSet::GetPhysicsLinear( btVector3* p_verts, unsigned int& numPVerts ) {
    if ( p_verts == NULL ) return;
    
    int width = GetWidth();
    int height = GetHeight();
    int numCubes = GetNumCubes();
    
    float radius = BLOCK_RADIUS;
    float r2 = BLOCK_RADIUS * 2.0f;
    
    int totalPVerts = 0;        // Physics verts
    
    for (int i=0; i<numCubes; i++) {
        Block& b = blocks[i];
        if ( b.blockType >= Type_Ice ) {
            int x = i >> (width_bits + height_bits);
            int y = (i >> width_bits) & ((1 << height_bits) - 1);
            int z = i & ((1 << width_bits) - 1);
            
            glm::vec3 offset = glm::vec3(radius-(width*radius), radius-(height*radius), radius-(width*radius));
//            glm::vec3 pos = cPos+glm::vec3(x*r2,y*r2,z*r2)+offset;
            glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+offset;
            
            // Add triangles to mesh
            bool lVis = !IsBlocked(x-1, y, z, b.blockType);
            bool rVis = !IsBlocked(x+1, y, z, b.blockType);
            bool bVis = !IsBlocked(x, y-1, z, b.blockType);
            bool tVis = !IsBlocked(x, y+1, z, b.blockType);
            bool reVis = !IsBlocked(x, y, z-1, b.blockType);
            bool fVis = !IsBlocked(x, y, z+1, b.blockType);
            
            if ( lVis ) {
                btVector3 lbr_p = btVector3(pos.x-radius, pos.y-radius, pos.z-radius);
                btVector3 lbf_p = btVector3(pos.x-radius, pos.y-radius, pos.z+radius);
                btVector3 ltr_p = btVector3(pos.x-radius, pos.y+radius, pos.z-radius);
                btVector3 ltf_p = btVector3(pos.x-radius, pos.y+radius, pos.z+radius);
                p_verts[totalPVerts++] = ltf_p;
                p_verts[totalPVerts++] = ltr_p;
                p_verts[totalPVerts++] = lbr_p;
                p_verts[totalPVerts++] = lbr_p;
                p_verts[totalPVerts++] = lbf_p;
                p_verts[totalPVerts++] = ltf_p;
            }
            if ( rVis ) {
                btVector3 rbr_p = btVector3(pos.x+radius, pos.y-radius, pos.z-radius);
                btVector3 rbf_p = btVector3(pos.x+radius, pos.y-radius, pos.z+radius);
                btVector3 rtr_p = btVector3(pos.x+radius, pos.y+radius, pos.z-radius);
                btVector3 rtf_p = btVector3(pos.x+radius, pos.y+radius, pos.z+radius);
                p_verts[totalPVerts++] = rtr_p;
                p_verts[totalPVerts++] = rtf_p;
                p_verts[totalPVerts++] = rbf_p;
                p_verts[totalPVerts++] = rbf_p;
                p_verts[totalPVerts++] = rbr_p;
                p_verts[totalPVerts++] = rtr_p;
            }
            if ( bVis ) {
                btVector3 lbr_p = btVector3(pos.x-radius, pos.y-radius, pos.z-radius);
                btVector3 lbf_p = btVector3(pos.x-radius, pos.y-radius, pos.z+radius);
                btVector3 rbr_p = btVector3(pos.x+radius, pos.y-radius, pos.z-radius);
                btVector3 rbf_p = btVector3(pos.x+radius, pos.y-radius, pos.z+radius);
                p_verts[totalPVerts++] = rbr_p;
                p_verts[totalPVerts++] = rbf_p;
                p_verts[totalPVerts++] = lbf_p;
                p_verts[totalPVerts++] = lbf_p;
                p_verts[totalPVerts++] = lbr_p;
                p_verts[totalPVerts++] = rbr_p;
            }
            if ( tVis ) {
                btVector3 ltr_p = btVector3(pos.x-radius, pos.y+radius, pos.z-radius);
                btVector3 ltf_p = btVector3(pos.x-radius, pos.y+radius, pos.z+radius);
                btVector3 rtr_p = btVector3(pos.x+radius, pos.y+radius, pos.z-radius);
                btVector3 rtf_p = btVector3(pos.x+radius, pos.y+radius, pos.z+radius);
                p_verts[totalPVerts++] = ltr_p;
                p_verts[totalPVerts++] = ltf_p;
                p_verts[totalPVerts++] = rtf_p;
                p_verts[totalPVerts++] = rtf_p;
                p_verts[totalPVerts++] = rtr_p;
                p_verts[totalPVerts++] = ltr_p;
            }
            if ( reVis ) {
                btVector3 lbr_p = btVector3(pos.x-radius, pos.y-radius, pos.z-radius);
                btVector3 ltr_p = btVector3(pos.x-radius, pos.y+radius, pos.z-radius);
                btVector3 rbr_p = btVector3(pos.x+radius, pos.y-radius, pos.z-radius);
                btVector3 rtr_p = btVector3(pos.x+radius, pos.y+radius, pos.z-radius);
                p_verts[totalPVerts++] = ltr_p;
                p_verts[totalPVerts++] = rtr_p;
                p_verts[totalPVerts++] = rbr_p;
                p_verts[totalPVerts++] = rbr_p;
                p_verts[totalPVerts++] = lbr_p;
                p_verts[totalPVerts++] = ltr_p;
            }
            if ( fVis ) {
                btVector3 lbf_p = btVector3(pos.x-radius, pos.y-radius, pos.z+radius);
                btVector3 ltf_p = btVector3(pos.x-radius, pos.y+radius, pos.z+radius);
                btVector3 rbf_p = btVector3(pos.x+radius, pos.y-radius, pos.z+radius);
                btVector3 rtf_p = btVector3(pos.x+radius, pos.y+radius, pos.z+radius);
                p_verts[totalPVerts++] = rtf_p;
                p_verts[totalPVerts++] = ltf_p;
                p_verts[totalPVerts++] = lbf_p;
                p_verts[totalPVerts++] = lbf_p;
                p_verts[totalPVerts++] = rbf_p;
                p_verts[totalPVerts++] = rtf_p;
            }
        }
    }
    numPVerts += totalPVerts;
//    changed = false;
}
void CubeSet::GetPhysicsReduced( btVector3* p_verts, unsigned int& numPVerts, float scale ) {
//    double timeStart = SysCore::GetMilliseconds();
    int width = GetWidth();
    int height = GetHeight();
    int merged = 0;
    bool vis = false;;
    int totalPVerts = 0;
    float radius = BLOCK_RADIUS*scale;
    float r2 = BLOCK_RADIUS * 2.0f*scale;
    //    glm::vec3 cPos = glm::vec3(coordinate.x*width*r2, -height*BLOCK_RADIUS, coordinate.y*width*r2);
    //    cPos += glm::vec3(BLOCK_RADIUS);
    glm::vec3 blockOffset = glm::vec3(radius-(width*radius), radius-(height*radius), radius-(width*radius));

    // View from negative x
    for(int x = width - 1; x >= 0; x--) {
        for(int y = 0; y < height; y++) {
            for(int z = 0; z < width; z++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                // Empty or line of sight blocked?
                if ( b.blockType < Type_Ice ||
                    PhysicsBlocked(x-1, y, z) ) {
                    vis = false;
                    continue;
                }
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
//                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2);
                btVector3 lbr_p = btVector3(pos.x-radius, pos.y-radius, pos.z-radius);
                btVector3 lbf_p = btVector3(pos.x-radius, pos.y-radius, pos.z+radius);
                btVector3 ltr_p = btVector3(pos.x-radius, pos.y+radius, pos.z-radius);
                btVector3 ltf_p = btVector3(pos.x-radius, pos.y+radius, pos.z+radius);
                if( vis && z != 0 ) {                   // Same block as previous one? Extend it.
                    p_verts[totalPVerts - 5] = lbf_p;
                    p_verts[totalPVerts - 2] = lbf_p;
                    p_verts[totalPVerts - 1] = ltf_p;
                    merged++;
                } else {                                // Otherwise, add a new quad.
                    p_verts[totalPVerts++] = lbr_p;
                    p_verts[totalPVerts++] = lbf_p;
                    p_verts[totalPVerts++] = ltr_p;
                    p_verts[totalPVerts++] = ltr_p;
                    p_verts[totalPVerts++] = lbf_p;
                    p_verts[totalPVerts++] = ltf_p;
                }
                vis = true;
            }
        }
    }
    // View from positive x
    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            for(int z = 0; z < width; z++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                if ( b.blockType < Type_Ice ||
                    PhysicsBlocked(x+1, y, z) ) {
                    vis = false;
                    continue;
                }
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                btVector3 rbr_p = btVector3(pos.x+radius, pos.y-radius, pos.z-radius);
                btVector3 rbf_p = btVector3(pos.x+radius, pos.y-radius, pos.z+radius);
                btVector3 rtr_p = btVector3(pos.x+radius, pos.y+radius, pos.z-radius);
                btVector3 rtf_p = btVector3(pos.x+radius, pos.y+radius, pos.z+radius);
                if(vis && z != 0 ) {
                    p_verts[totalPVerts - 4] = rbf_p;
                    p_verts[totalPVerts - 2] = rtf_p;
                    p_verts[totalPVerts - 1] = rbf_p;
                    merged++;
                } else {
                    p_verts[totalPVerts++] = rbr_p;
                    p_verts[totalPVerts++] = rtr_p;
                    p_verts[totalPVerts++] = rbf_p;
                    p_verts[totalPVerts++] = rtr_p;
                    p_verts[totalPVerts++] = rtf_p;
                    p_verts[totalPVerts++] = rbf_p;
                }
                vis = true;
            }
        }
    }
    // View from negative y
    for(int x = 0; x < width; x++) {
        for(int y = height - 1; y >= 0; y--) {
            for(int z = 0; z < width; z++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                if ( b.blockType < Type_Ice ||
                    PhysicsBlocked(x, y-1, z) ) {
                    vis = false;
                    continue;
                }
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                btVector3 lbr_p = btVector3(pos.x-radius, pos.y-radius, pos.z-radius);
                btVector3 lbf_p = btVector3(pos.x-radius, pos.y-radius, pos.z+radius);
                btVector3 rbr_p = btVector3(pos.x+radius, pos.y-radius, pos.z-radius);
                btVector3 rbf_p = btVector3(pos.x+radius, pos.y-radius, pos.z+radius);
                if(vis && z != 0 ) {
                    p_verts[totalPVerts - 4] = lbf_p;
                    p_verts[totalPVerts - 2] = rbf_p;
                    p_verts[totalPVerts - 1] = lbf_p;
                    merged++;
                } else {
                    p_verts[totalPVerts++] = lbr_p;
                    p_verts[totalPVerts++] = rbr_p;
                    p_verts[totalPVerts++] = lbf_p;
                    p_verts[totalPVerts++] = rbr_p;
                    p_verts[totalPVerts++] = rbf_p;
                    p_verts[totalPVerts++] = lbf_p;
                }
                vis = true;
            }
        }
    }
    // View from positive y
    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            for(int z = 0; z < width; z++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                // Empty or line of sight blocked?
                if ( b.blockType < Type_Ice ||
                    PhysicsBlocked(x, y+1, z) ) {
                    vis = false;
                    continue;
                }
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                btVector3 ltr_p = btVector3(pos.x-radius, pos.y+radius, pos.z-radius);
                btVector3 ltf_p = btVector3(pos.x-radius, pos.y+radius, pos.z+radius);
                btVector3 rtr_p = btVector3(pos.x+radius, pos.y+radius, pos.z-radius);
                btVector3 rtf_p = btVector3(pos.x+radius, pos.y+radius, pos.z+radius);
                if(vis && z != 0 ) {
                    p_verts[totalPVerts - 5] = ltf_p;
                    p_verts[totalPVerts - 2] = ltf_p;
                    p_verts[totalPVerts - 1] = rtf_p;
                    merged++;
                } else {
                    p_verts[totalPVerts++] = ltr_p;
                    p_verts[totalPVerts++] = ltf_p;
                    p_verts[totalPVerts++] = rtr_p;
                    p_verts[totalPVerts++] = rtr_p;
                    p_verts[totalPVerts++] = ltf_p;
                    p_verts[totalPVerts++] = rtf_p;
                }
                vis = true;
            }
        }
    }
    // View from negative z
    for(int x = 0; x < width; x++) {
        for(int z = width - 1; z >= 0; z--) {
            for(int y = 0; y < height; y++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                // Empty or line of sight blocked?
                if ( b.blockType < Type_Ice ||
                    PhysicsBlocked(x, y, z-1) ) {
                    vis = false;
                    continue;
                }
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                btVector3 lbr_p = btVector3(pos.x-radius, pos.y-radius, pos.z-radius);
                btVector3 ltr_p = btVector3(pos.x-radius, pos.y+radius, pos.z-radius);
                btVector3 rbr_p = btVector3(pos.x+radius, pos.y-radius, pos.z-radius);
                btVector3 rtr_p = btVector3(pos.x+radius, pos.y+radius, pos.z-radius);
                if(vis && y != 0 ) {
                    p_verts[totalPVerts - 5] = ltr_p;
                    p_verts[totalPVerts - 3] = ltr_p;
                    p_verts[totalPVerts - 2] = rtr_p;
                    merged++;
                } else {
                    p_verts[totalPVerts++] = lbr_p;
                    p_verts[totalPVerts++] = ltr_p;
                    p_verts[totalPVerts++] = rbr_p;
                    p_verts[totalPVerts++] = ltr_p;
                    p_verts[totalPVerts++] = rtr_p;
                    p_verts[totalPVerts++] = rbr_p;
                }
                vis = true;
            }
        }
    }
    // View from positive z
    for(int x = 0; x < width; x++) {
        for(int z = 0; z < width; z++) {
            for(int y = 0; y < height; y++) {
                int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                Block& b = blocks[arrPos];
                // Empty or line of sight blocked?
                if ( b.blockType < Type_Ice ||
                    PhysicsBlocked(x, y, z+1) ) {
                    vis = false;
                    continue;
                }
                glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+blockOffset;
                btVector3 lbf_p = btVector3(pos.x-radius, pos.y-radius, pos.z+radius);
                btVector3 ltf_p = btVector3(pos.x-radius, pos.y+radius, pos.z+radius);
                btVector3 rbf_p = btVector3(pos.x+radius, pos.y-radius, pos.z+radius);
                btVector3 rtf_p = btVector3(pos.x+radius, pos.y+radius, pos.z+radius);
                if(vis && y != 0 ) {
                    p_verts[totalPVerts - 4] = ltf_p;
                    p_verts[totalPVerts - 3] = ltf_p;
                    p_verts[totalPVerts - 1] = rtf_p;
                    merged++;
                } else {
                    p_verts[totalPVerts++] = lbf_p;
                    p_verts[totalPVerts++] = rbf_p;
                    p_verts[totalPVerts++] = ltf_p;
                    p_verts[totalPVerts++] = ltf_p;
                    p_verts[totalPVerts++] = rbf_p;
                    p_verts[totalPVerts++] = rtf_p;
                }
                vis = true;
            }
        }
    }
    numPVerts += totalPVerts;
}
void CubeSet::GetPhysicsCubes( btCompoundShape *shape, const float scale ) {
    int width = GetWidth();
    int height = GetHeight();
    int numCubes = GetNumCubes();
    float radius = BLOCK_RADIUS*scale;
    float r2 = BLOCK_RADIUS * 2.0f * scale;

    for (int i=0; i<numCubes; i++) {
        Block& b = blocks[i];
        int blockType = b.blockType;
        if ( blockType >= Type_Dirt ) {
            int x = i >> (width_bits + height_bits);
            int y = (i >> width_bits) & ((1 << height_bits) - 1);
            int z = i & ((1 << width_bits) - 1);
            glm::vec3 offset = glm::vec3(radius-(width*radius), radius-(height*radius), radius-(width*radius));
            glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+offset;
            // Add triangles to mesh
            if ( PhysicsBlocked(x-1, y, z) &&
                PhysicsBlocked(x+1, y, z) &&
                PhysicsBlocked(x, y-1, z) &&
                PhysicsBlocked(x, y+1, z) &&
                PhysicsBlocked(x, y, z-1) &&
                PhysicsBlocked(x, y, z+1) )  {
                continue;
            }
            btBoxShape* box = new btBoxShape(btVector3(radius, radius, radius));
            btTransform trans = btTransform();
            trans.setIdentity();
            trans.setOrigin( btVector3(pos.x, pos.y, pos.z) );
            shape->addChildShape(trans, box);
        }
    }
}
void CubeSet::GetPhysicsHull( btConvexHullShape *shape, const float scale ) {
    int width = GetWidth();
    int height = GetHeight();
    int numCubes = GetNumCubes();
    float radius = BLOCK_RADIUS*scale;
    float r2 = BLOCK_RADIUS * 2.0f * scale;
    
    for (int i=0; i<numCubes; i++) {
        Block& b = blocks[i];
        if ( b.blockType >= Type_Dirt ) {
            int x = i >> (width_bits + height_bits);
            int y = (i >> width_bits) & ((1 << height_bits) - 1);
            int z = i & ((1 << width_bits) - 1);
            
            glm::vec3 offset = glm::vec3(radius-(width*radius), radius-(height*radius), radius-(width*radius));
            glm::vec3 pos = glm::vec3(x*r2,y*r2,z*r2)+offset;
            // Add triangles to mesh
            if ( PhysicsBlocked(x-1, y, z) &&
                PhysicsBlocked(x+1, y, z) &&
                PhysicsBlocked(x, y-1, z) &&
                PhysicsBlocked(x, y+1, z) &&
                PhysicsBlocked(x, y, z-1) &&
                PhysicsBlocked(x, y, z+1) )  {
                continue;
            }
            btVector3 lbr_p = btVector3(pos.x-radius, pos.y-radius, pos.z-radius);
            btVector3 lbf_p = btVector3(pos.x-radius, pos.y-radius, pos.z+radius);
            btVector3 ltr_p = btVector3(pos.x-radius, pos.y+radius, pos.z-radius);
            btVector3 ltf_p = btVector3(pos.x-radius, pos.y+radius, pos.z+radius);
            btVector3 rbr_p = btVector3(pos.x+radius, pos.y-radius, pos.z-radius);
            btVector3 rbf_p = btVector3(pos.x+radius, pos.y-radius, pos.z+radius);
            btVector3 rtr_p = btVector3(pos.x+radius, pos.y+radius, pos.z-radius);
            btVector3 rtf_p = btVector3(pos.x+radius, pos.y+radius, pos.z+radius);
            shape->addPoint(lbr_p);
            shape->addPoint(lbf_p);
            shape->addPoint(ltr_p);
            shape->addPoint(ltf_p);
            shape->addPoint(rbr_p);
            shape->addPoint(rbf_p);
            shape->addPoint(rtr_p);
            shape->addPoint(rtf_p);
        }
    }
}
void CubeSet::GetPhysicsAABBs( btCompoundShape *shape, const float scale ) {
    std::vector<VoxelAABB*> aabbVect;       // Final AABBs
    std::vector<VoxelAABB*> aabbVectXPrev;  // Previous pass of X AABBs
    
    int width = GetWidth();
    int height = GetHeight();
    float radius = BLOCK_RADIUS*scale;
    float r2 = BLOCK_RADIUS * 2.0f * scale;
    float epsilon = BLOCK_RADIUS*0.1f;
    
    for ( int px=0; px < GetWidth(); px++ ) {
        std::vector<VoxelAABB*> aabbVectX;      // This pass of X AABBs
        std::vector<VoxelAABB*> aabbVectYPrev;  // Previous pass of Y AABBs
        for ( int py=0; py < GetHeight(); py++ ) {
            std::vector<VoxelAABB*> aabbVectY;  // This pass of Y AABBs
            BlockType prevZ = Type_Empty;
            for ( int pz=0; pz < GetWidth(); pz++ ) {
                int arrIndex = GetIndex(px,py,pz, width_bits, height_bits);
                Block& bz = blocks[arrIndex];
                if ( bz.blockType != Type_Empty ) {
                    glm::vec3 offset = glm::vec3(radius-(width*radius), radius-(height*radius), radius-(width*radius));
                    glm::vec3 pos = (glm::vec3(px,py,pz)*r2)+offset;
                    VoxelAABB* aabb = NULL;
                    if ( prevZ == Type_Empty ) {
                        aabb = new VoxelAABB(pos-glm::vec3(radius),
                                             pos+glm::vec3(radius),
                                             bz.blockType);
                        aabbVectY.push_back(aabb);
                    } else {                // Same type, extend AABB
                        aabb = aabbVectY.back();
                        aabb->m_max.z = pos.z+radius;
                    }
                    // Compare current new y aabbs with previous set
                    for (int p = 0; p < aabbVectYPrev.size(); p++) {
                        VoxelAABB * prev = aabbVectYPrev[p];
                        if (fabsf(prev->m_min.x - aabb->m_min.x) < epsilon &&
                            fabsf(prev->m_max.x - aabb->m_max.x) < epsilon &&
                            fabsf(prev->m_min.z - aabb->m_min.z) < epsilon &&
                            fabsf(prev->m_max.z - aabb->m_max.z) < epsilon &&
                            fabsf(prev->m_max.y - aabb->m_min.y) < epsilon ) {
                            aabb->m_min.y = prev->m_min.y;
                            aabbVectYPrev.erase(aabbVectYPrev.begin()+p);
                            delete prev;
                            prevZ = Type_Empty;    // Must not merge with this one next
                            break;
                        }
                    }
                }
                prevZ = bz.blockType;
            }   // For each z
            if ( aabbVectYPrev.size() != 0 ) {  // Copy previous Y aabbs to final vect
                std::copy(aabbVectYPrev.begin(), aabbVectYPrev.end(), std::back_inserter(aabbVectX));
                aabbVectYPrev.clear();
            }
            if ( aabbVectY.size() != 0 ) {      // Copy previous Y aabbs to previous vect
                std::copy(aabbVectY.begin(), aabbVectY.end(), std::back_inserter(aabbVectYPrev));
                aabbVectY.clear();
            }
            if ( py == GetHeight()-1 ) {         // Copy previous Y aabbs to final vect (in case left over)
                if ( aabbVectYPrev.size() != 0 ) {
                    std::copy(aabbVectYPrev.begin(), aabbVectYPrev.end(), std::back_inserter(aabbVectX));
                    aabbVectYPrev.clear();
                }
            }
        }   // For each y
        // Compare new X aabbs with previous set
        for (int pXP = 0; pXP < aabbVectXPrev.size(); pXP++) {
            for (int pX = 0; pX < aabbVectX.size(); pX++) {
				if (pXP >= aabbVectXPrev.size() ) continue;
                VoxelAABB * prev = aabbVectXPrev[pXP];
                VoxelAABB * aabb = aabbVectX[pX];
                if (fabsf(prev->m_min.y - aabb->m_min.y) < epsilon &&
                    fabsf(prev->m_max.y - aabb->m_max.y) < epsilon &&
                    fabsf(prev->m_min.z - aabb->m_min.z) < epsilon &&
                    fabsf(prev->m_max.z - aabb->m_max.z) < epsilon &&
                    fabsf(prev->m_max.x - aabb->m_min.x) < epsilon ) {
                    aabb->m_min.x = prev->m_min.x;
                    aabbVectXPrev.erase(aabbVectXPrev.begin()+pXP);
                    delete prev;
			
                }
            }
        }
        if ( aabbVectXPrev.size() != 0 ) {  // Copy previous X aabbs to final vect
            std::copy(aabbVectXPrev.begin(), aabbVectXPrev.end(), std::back_inserter(aabbVect));
            aabbVectXPrev.clear();
        }
        if ( aabbVectX.size() != 0 ) {      // Copy current X aabbs to previous vect
            std::copy(aabbVectX.begin(), aabbVectX.end(), std::back_inserter(aabbVectXPrev));
            aabbVectX.clear();
        }
        if ( px == GetWidth()-1 ) {
            if ( aabbVectXPrev.size() != 0 ) {  // Copy previous X aabbs to final vect (in case left over)
                std::copy(aabbVectXPrev.begin(), aabbVectXPrev.end(), std::back_inserter(aabbVect));
                aabbVectXPrev.clear();
            }
        }
    }   // For each x
    
    // Create physics shapes from AABBs
    std::vector<VoxelAABB*>::iterator
    it = aabbVect.begin();
    while ( it != aabbVect.end() ) {
        VoxelAABB* aabb = *it;
        BlockType b = aabb->m_voxel;
        if ( b >= Type_Ice ) {
            glm::vec3 aaBBSize_2 = (aabb->m_max-aabb->m_min)*0.5f;
            glm::vec3 pos = aabb->m_min+aaBBSize_2;
            btBoxShape* box = new btBoxShape(btVector3(aaBBSize_2.x, aaBBSize_2.y, aaBBSize_2.z));
            btTransform trans = btTransform();
            trans.setIdentity();
            trans.setOrigin( btVector3(pos.x, pos.y, pos.z) );
            shape->addChildShape(trans, box);
        }
        it++;
        delete aabb;
    }
    aabbVect.clear();
}
void CubeSet::GetDynaBlocks( World3D *world, const glm::vec3 position, const float scale ) {
    int width = GetWidth();
    int height = GetHeight();
    int numCubes = GetNumCubes();
    float radius = BLOCK_RADIUS*scale;
    float r2 = BLOCK_RADIUS * 2.0f * scale;
    btVector3 halfSize = btVector3(radius,radius,radius);
    btVector3 move = btVector3(position.x,position.y,position.z);
    for (int i=0; i<numCubes; i++) {
        Block& b = blocks[i];
        if ( b.blockType >= Type_Dirt ) {
            int x = i >> (width_bits + height_bits);
            int y = (i >> width_bits) & ((1 << height_bits) - 1);
            int z = i & ((1 << width_bits) - 1);
            btVector3 offset = btVector3(radius-(width*radius), radius-(height*radius), radius-(width*radius));
            btVector3 pos = btVector3(x*r2,y*r2,z*r2)+offset+move;
            DynaCube* cube = world->AddDynaCube(pos, halfSize, ColorForType(BlockType(b.blockType)));
            if ( cube ) cube->timer = 5.0f;
        }
    }
}
inline bool CubeSet::GetVisibility( const int x, const int y, const int z ) {
//    int width = (1<<width_bits);
//    int height = (1<<height_bits);
    int xMove = 0, yMove = 0, zMove = 0;
    Block* b = NULL;
    
    // Check if within bounds
    if ( x < 0 ) { xMove = -1; }
    else if ( x > GetWidth()-1 ) { xMove = 1; }
    if ( z < 0 ) { zMove = -1; }
    else if ( z > GetWidth()-1 ) { zMove = 1; }
    if ( y < 0 ) { yMove = -1; }
    else if ( y > GetHeight()-1 ) { yMove = 1; }
    if ( yMove != 0 ) { return true; }
    else if ( xMove !=0 || zMove != 0 ) {
        CubeSet* neighbor = NULL;
        if ( xMove == -1 ) {
            if ( zMove == -1 ) neighbor = neighborLB;
            else if ( zMove == 1 ) neighbor = neighborLF;
            else neighbor = neighborL;
        } else if ( xMove == 1 ) {
            if ( zMove == -1 ) neighbor = neighborRB;
            else if ( zMove == 1 ) neighbor = neighborRF;
            else neighbor = neighborR;
        } else {
            if ( zMove == -1 ) neighbor = neighborB;
            if ( zMove == 1 ) neighbor = neighborF;
        }
        if ( neighbor ) {
            b = &neighbor->Get(x-((GetWidth())*xMove), y, z-((GetWidth())*zMove));
        } else return true;
    } else {
        int arrPos = GetIndex(x, y, z, width_bits, height_bits);
        b = &blocks[arrPos];
    }
    if ( b->blockType >= Type_Dirt ) return false;
    return true;
}
bool CubeSet::IsBlocked( const int x, const int y, int z, const int type ) {
    int xMove = 0, yMove = 0, zMove = 0;
    Block* b = NULL;
    // Check if within bounds
    if ( x < 0 ) { xMove = -1; }
    else if ( x > GetWidth()-1 ) { xMove = 1; }
    if ( z < 0 ) { zMove = -1; }
    else if ( z > GetWidth()-1 ) { zMove = 1; }
    if ( y < 0 ) { yMove = -1; }
    else if ( y > GetHeight()-1 ) { yMove = 1; }
    
    if ( yMove != 0 ) { return true; }
    else if ( xMove !=0 || zMove != 0 ) {
        CubeSet* neighbor = NULL;
        if ( xMove == -1 ) {
            if ( zMove == -1 ) neighbor = neighborLB;
            else if ( zMove == 1 ) neighbor = neighborLF;
            else neighbor = neighborL;
        } else if ( xMove == 1 ) {
            if ( zMove == -1 ) neighbor = neighborRB;
            else if ( zMove == 1 ) neighbor = neighborRF;
            else neighbor = neighborR;
        } else {
            if ( zMove == -1 ) neighbor = neighborB;
            if ( zMove == 1 ) neighbor = neighborF;
        }
        if ( neighbor ) {
            b = &neighbor->Get(x-((GetWidth())*xMove), y, z-((GetWidth())*zMove));
        } else return true;
    } else {
        int arrPos = GetIndex(x, y, z, width_bits, height_bits);
        b = &blocks[arrPos];
    }

    if ( b->blockType >= Type_Ice ) return true;
    
    // Invisible blocks are always "blocked"
    if( type == Type_Empty )
        return true;
    // No neighbor blocking the way
    if ( b->blockType == Type_Empty )
        return false;
    // Leaves do not block any other block, including themselves
//    if( b->blockType == Type_Leaves_1 )
//        return false;
    // Non-transparent blocks always block line of sight
    if( b->blockType >= Type_Dirt)
        return true;
    // Water is a special case
    if ( type == Type_Water ) {
        if ( b->blockType == Type_Empty || b->blockType != Type_Water ) return false;
        else return true;
    }
    // Otherwise, LOS is only blocked by blocks if the same transparency type
    return ( b->blockType == type );
}
bool CubeSet::IsBlocked( const int x, const int y, const int z, const int type,
                        std::function<class CubeSet*(int, int)> neighborFunc) {
    int width = (1<<width_bits);
    int height = (1<<height_bits);
    int arrPos = 0;
    Block* b=NULL;
    // Check if within bounds
    if ( y < 0 || y > height-1 ) { return  false; }
    else if (x < 0 || x > width-1 ||
        z < 0 || z > width-1 ) {
        int xMove = 0;
        int zMove = 0;
        if ( x < 0 ) { xMove = -1; }
        else if ( x > width-1 ) { xMove = 1; }
        if ( z < 0 ) { zMove = -1; }
        else if ( z > width-1 ) { zMove = 1; }
        if ( neighborFunc ) {
            CubeSet* neighbor = neighborFunc(xMove,zMove);
            if ( neighbor ) {
                arrPos = GetIndex(x-((width-1)*xMove), y, z-((width-1)*zMove), width_bits, height_bits);
                b = &neighbor->Get(x-((width-1)*xMove), y, z-((width-1)*zMove));
            } else return false;
        } else return false;
    } else {
        arrPos = GetIndex(x, y, z, width_bits, height_bits);
        b = &blocks[arrPos];
    }

    if ( b->blockType >= Type_Dirt ) return true;
    
    // Invisible blocks are always "blocked"
    if( type == Type_Empty )
        return true;
    // No neighbor blocking the way
    if ( b->blockType == Type_Empty )
        return false;
    // Leaves do not block any other block, including themselves
//    if( b->blockType == Type_Leaves )
//        return false;
    // Non-transparent blocks always block line of sight
    if( b->blockType >= Type_Dirt)
        return true;
    // Water is a special case
    if ( type == Type_Water ) {
        if ( b->blockType == Type_Empty || b->blockType != Type_Water ) return false;
        else return true;
    }
    // Otherwise, LOS is only blocked by blocks if the same transparency type
    return ( b->blockType == type );
}
bool CubeSet::IsBlocked( const int x, const int y, const int z, const int type,
                        CubeSet* neighbor ) {
    int width = (1<<width_bits);
    int height = (1<<height_bits);
    int arrPos = 0;
    Block* b=NULL;
    // Check if within bounds
    if ( y < 0 || y > height-1 ) { return  false; }
    else if (x < 0 || x > width-1 ||
             z < 0 || z > width-1 ) {
        int xMove = 0;
        int zMove = 0;
        if ( x < 0 ) { xMove = -1; }
        else if ( x > width-1 ) { xMove = 1; }
        if ( z < 0 ) { zMove = -1; }
        else if ( z > width-1 ) { zMove = 1; }
        if ( neighbor ) {
            arrPos = GetIndex(x-((width-1)*xMove), y, z-((width-1)*zMove), width_bits, height_bits);
            b = &neighbor->Get(x-((width-1)*xMove), y, z-((width-1)*zMove));
        } else return false;
    } else {
        arrPos = GetIndex(x, y, z, width_bits, height_bits);
        b = &blocks[arrPos];
    }
    
    if ( b->blockType >= Type_Dirt ) return true;
    
    // Invisible blocks are always "blocked"
    if( type == Type_Empty )
        return true;
    // No neighbor blocking the way
    if ( b->blockType == Type_Empty )
        return false;
    // Leaves do not block any other block, including themselves
//    if( b->blockType == Type_Leaves )
//        return false;
    // Non-transparent blocks always block line of sight
    if( b->blockType >= Type_Dirt)
        return true;
    // Water is a special case
    if ( type == Type_Water ) {
        if ( b->blockType == Type_Empty || b->blockType != Type_Water ) return false;
        else return true;
    }
    // Otherwise, LOS is only blocked by blocks if the same transparency type
    return ( b->blockType == type );
}

bool CubeSet::PhysicsBlocked( int x, int y, int z ) {
    Block* b = NULL;
    int xMove = 0;
    int yMove = 0;
    int zMove = 0;
    if ( x < 0 ) { xMove = -1; }
    else if ( x > GetWidth()-1 ) { xMove = 1; }
    if ( z < 0 ) { zMove = -1; }
    else if ( z > GetWidth()-1 ) { zMove = 1; }
    if ( y < 0 ) { yMove = -1; }
    else if ( y > GetHeight()-1 ) { yMove = 1; }
    if ( yMove != 0 ) { return false; }
    else if ( xMove !=0 || zMove != 0 ) {
        CubeSet* neighbor = NULL;
        if ( xMove == -1 ) {
            if ( zMove == -1 ) neighbor = neighborLB;
            else if ( zMove == 1 ) neighbor = neighborLF;
            else neighbor = neighborL;
        } else if ( xMove == 1 ) {
            if ( zMove == -1 ) neighbor = neighborRB;
            else if ( zMove == 1 ) neighbor = neighborRF;
            else neighbor = neighborR;
        } else {
            if ( zMove == -1 ) neighbor = neighborB;
            if ( zMove == 1 ) neighbor = neighborF;
        }
        if ( neighbor ) {
            b = &neighbor->Get(x-((GetWidth())*xMove), y, z-((GetWidth())*zMove));
        } else return false;
    } else {
        b = &Get(x, y, z);
    }
    // Non-transparent blocks always block line of sight
    if( b->blockType != Type_Empty ) { return true; }
    return false;
}
bool CubeSet::ColorBlocked( int x, int y, int z, Color col ) {
    Block* b = NULL;
    int xMove = 0;
    int yMove = 0;
    int zMove = 0;
    if ( x < 0 ) { xMove = -1; }
    else if ( x > GetWidth()-1 ) { xMove = 1; }
    if ( z < 0 ) { zMove = -1; }
    else if ( z > GetWidth()-1 ) { zMove = 1; }
    if ( y < 0 ) { yMove = -1; }
    else if ( y > GetHeight()-1 ) { yMove = 1; }
    if ( yMove != 0 ) { return true; }
    else if ( xMove !=0 || zMove != 0 ) {
        CubeSet* neighbor = NULL;
        if ( xMove == -1 ) {
            if ( zMove == -1 ) neighbor = neighborLB;
            else if ( zMove == 1 ) neighbor = neighborLF;
            else neighbor = neighborL;
        } else if ( xMove == 1 ) {
            if ( zMove == -1 ) neighbor = neighborRB;
            else if ( zMove == 1 ) neighbor = neighborRF;
            else neighbor = neighborR;
        } else {
            if ( zMove == -1 ) neighbor = neighborB;
            if ( zMove == 1 ) neighbor = neighborF;
        }
        if ( neighbor ) {
            b = &neighbor->Get(x-((GetWidth())*xMove), y, z-((GetWidth())*zMove));
        } else return true;
    } else {
        b = &Get(x, y, z);
    }
    if ( b == NULL ) return true;
    // Non-transparent blocks always block line of sight
    return ( b->blockColor == col );
}
void CubeSet::GenerateTree( const glm::vec3 treePos, const int seed ) {
    Random::RandomSeed(Timer::Seconds());
//    int height = (1<<height_bits-1);
    
    
    // TODO:: Make tree variables editable
    float trunkHeight = 8.0f;
    float trunkRadius = BLOCK_RADIUS*2.0f;
    float leafRadius = 1.0f;
    float blockWidth = BLOCK_RADIUS*2.0f;

    // Debug simple tree
//    AddSphere(glm::vec3(0.0f, -(height*BLOCK_RADIUS), 0.0f), glm::vec3(trunkRadius, trunkHeight, trunkRadius), Type_Tree );
//    AddSphere(glm::vec3(0.0f, -(height*BLOCK_RADIUS)+leafRadius, 0.0f), glm::vec3(leafRadius, leafRadius, leafRadius), Type_Grass );
    
    struct Branch {
        glm::vec3 pos;
        glm::vec3 end;
        float radius;
    };
    std::vector<Branch> branches;
    int maxBranches = 4;
//    Branch trunk = { glm::vec3(0.0f,-height*BLOCK_RADIUS*2,0.0f), glm::vec3(0.0f, trunkHeight, 0.0f), trunkRadius };
    Branch trunk = { treePos, treePos+glm::vec3(0.0f, trunkHeight, 0.0f), trunkRadius };

    branches.push_back(trunk);
    // Generate trunk
    for (int i=0; i<maxBranches; i++) {
        if ( i > branches.size()-1 ) break;
        // Process branch
        Branch b = branches[i];
        glm::vec3 newPos = b.pos;
        float finishDist = glm::distance(b.pos, b.end);
        bool done = false;
        while (!done) {
            bool branched = false;
            int sides_b = b.radius/BLOCK_RADIUS;
            if ( sides_b < 1 ) {
                int arrPos = PosToArray(newPos);
                if ( blocks[arrPos].blockType == Type_Empty ) {
                    blocks[arrPos].blockType = Type_Wood_1;
                    blocks[arrPos].blockColor = ColorForType(Type_Wood_1);
                    if ( i==0 ) {
                        float trunkCol = 0.8f;
                        blocks[arrPos].blockColor.r *= trunkCol;
                        blocks[arrPos].blockColor.g *= trunkCol;
                        blocks[arrPos].blockColor.b *= trunkCol;
                    }
                }
            } else {
                // Add blocks for current branch position
                for (int x=-sides_b; x<sides_b; x++) {
                    for (int z=-sides_b; z<sides_b; z++) {
                        for (int y=-sides_b; y<sides_b; y++) {
                            float posX = newPos.x+(x*blockWidth)+BLOCK_RADIUS;
                            float posZ = newPos.z+(z*blockWidth)+BLOCK_RADIUS;
                            float posY = newPos.y+(y*blockWidth)+BLOCK_RADIUS;
                            glm::vec3 cubePos = glm::vec3(posX, posY, posZ);
                            if ( glm::distance(cubePos, newPos) < b.radius ) {
                                int arrPos = PosToArray(cubePos);
                                blocks[arrPos].blockType = Type_Wood_1;
                                blocks[arrPos].blockColor = ColorForType(Type_Wood_1);
                            }
                        }   // for y
                    }   // for z
                }   // for x
            }
            
//        }   // while !done
//    }   // for i < maxBranches
//            if ( glm::distance(glm::vec2(treePos.x,treePos.z),glm::vec2(newPos.x,newPos.z)) > GetWidth()*BLOCK_RADIUS*0.5f ) done = true;
            // Update branch position
            glm::vec3 move = glm::normalize(b.end-newPos)*float(BLOCK_RADIUS);
            if ( newPos.y+move.y < b.end.y ) {
                float finishRatio = glm::distance(newPos, b.end)/finishDist;
                // Shrink radius
                double shrinkSize = Random::RandomDouble()*(1.0-finishRatio)*0.025*b.radius;
                b.radius -= shrinkSize;
                if ( b.radius < BLOCK_RADIUS*0.5f ) { done = true; }
                // Check if we want to finish the branch
                double randomFinish = Random::RandomDouble()*0.1f;
                if ( finishRatio - randomFinish < 0.0f ) {
                    // Split at random finish
                    if ( branches.size() < maxBranches ) {
                        float newBranchSize = b.radius*0.5f;
                        double newBranchX = (Random::RandomDouble()*2.0-1.0);
                        double newBranchY = (Random::RandomDouble()*2.0-1.0);
                        double newBranchZ = (Random::RandomDouble()*2.0-1.0);
                        glm::vec3 newEnd = glm::normalize(glm::vec3(newBranchX, newBranchY, newBranchZ))*finishDist*0.6f;
                        Branch newBranch = { newPos, newEnd, newBranchSize };
                        branches.push_back(newBranch);
                        glm::vec3 newEnd2 = glm::normalize(glm::vec3(-newBranchX, newBranchY, -newBranchZ))*finishDist*0.6f;
                        Branch newBranch2 = { newPos, newEnd2, newBranchSize };
                        branches.push_back(newBranch2);
                        branched = true;
                    }
                    done = true;
                }
                // Check if we want to deviate course
                double randomX = (Random::RandomInt(-1, 1))*BLOCK_RADIUS*0.5f;
                double randomZ = randomX == 0 ? (Random::RandomInt(-1, 1))*BLOCK_RADIUS*0.5f : 0;
                move.x += randomX;
                move.z += randomZ;
                // Check if we want to branch off
                if ( branches.size() < maxBranches ) {
                    double newBranchRand = Random::RandomDouble()*(1.0-finishRatio);
                    if ( newBranchRand > 0.5f || done ) {
                        float newBranchSize = b.radius*0.5f;
                        double newBranchX = (Random::RandomDouble()*2.0-1.0);
                        double newBranchY = (Random::RandomDouble()*2.0-1.0);
                        double newBranchZ = (Random::RandomDouble()*2.0-1.0);
                        glm::vec3 newEnd = glm::normalize(glm::vec3(newBranchX, newBranchY, newBranchZ))*finishDist*0.3f;
                        Branch newBranch = { newPos, newEnd, newBranchSize };
                        branches.push_back(newBranch);
                        branched = true;
                    }
                }
            } else {
                done = true;
            }
            // Generate leaves
            if ( done && !branched ) {
                double foliageX = leafRadius+(Random::RandomInt(1, 2))*blockWidth;
                double foliageY = leafRadius+(Random::RandomInt(1, 2))*blockWidth;
                // Create foliage at end of branch
                SetSphere(newPos, glm::vec3(foliageX, foliageY, foliageX), Type_Grass, false);
            }
            newPos += move;
        }   // while !done
    }   // for i < maxBranches
    
    changed = true;
}
void CubeSet::GenerateGrass( const int seed ) {
    for (int x=0; x<GetWidth(); x++) {
        for (int z=0; z<GetWidth(); z++) {
            float threshold = glm::simplex(glm::vec3(x*0.5f,seed,z*0.5f));
            if ( threshold > 0.3f ) {
                int height = threshold*GetHeight();
                for (int y=0; y < height; y++) {
                    int arrPos = GetIndex(x, y, z, width_bits, height_bits);
                    if ( arrPos != -1 ) {
                        blocks[arrPos].blockType = Type_Grass;
                        blocks[arrPos].blockColor = ColorForType(Type_Grass);
//                        blocks[arrPos].blockColor.r += (float(height-y)/height)*threshold*0.5f;
//                        blocks[arrPos].blockColor.g += (y/height)*0.04f;
                    }
                }
            }
        }
    }
    changed = true;
}
void CubeSet::SetVolume( const AABB3D volume, const BlockType type, const bool replace ) {
    const float blockWidth = BLOCK_RADIUS*2.0f;
    const glm::vec3 volSize = volume.GetVolume();
    // Number of blocks per axis
    int blocksX = int_clamp(volSize.x/blockWidth, 1, GetWidth());
    int blocksY = int_clamp(volSize.y/blockWidth, 1, GetWidth());
    int blocksZ = int_clamp(volSize.z/blockWidth, 1, GetWidth());
    // Add blocks inside volume
    for (int x=0; x < blocksX; x++) {
        for (int y=0; y < blocksY; y++) {
            for (int z=0; z < blocksZ; z++) {
                glm::vec3 cubePos = volume.m_min+(glm::vec3(x,y,z)*blockWidth);
                int arrPos = PosToArray(cubePos);
                if ( arrPos != -1 ) {
                    if ( replace || blocks[arrPos].blockType == Type_Empty ) {
                        blocks[arrPos].blockType = type;
                        blocks[arrPos].blockColor = ColorForType(type);
                    }
                } else {
//                    printf("[CubeSet] Volume failed, no block %i\n", arrPos);
                }
            } // for z
        } // for y
    } // for x
    changed = true;
}
void CubeSet::SetVolume( const glm::vec3 center, const glm::vec3 radius, const BlockType type, const bool replace ) {
    const float blockWidth = BLOCK_RADIUS*2.0f;
    glm::vec3 min = center-radius;
    // Number of blocks per axis
    int blocksX = int_clamp(radius.x/BLOCK_RADIUS, 1, GetWidth());
    int blocksY = int_clamp(radius.y/BLOCK_RADIUS, 1, GetWidth());
    int blocksZ = int_clamp(radius.z/BLOCK_RADIUS, 1, GetWidth());
    // Add blocks inside volume
    for (int x=0; x <= blocksX; x++) {
        for (int y=0; y <= blocksY; y++) {
            for (int z=0; z <= blocksZ; z++) {
                glm::vec3 cubePos = min+(glm::vec3(x,y,z)*blockWidth);
                int arrPos = PosToArray(cubePos);
                if ( arrPos != -1 ) {
                    if ( replace || blocks[arrPos].blockType == Type_Empty ) {
                        blocks[arrPos].blockType = type;
                        blocks[arrPos].blockColor = ColorForType(type);
                    }
                } else {
//                    printf("[CubeSet] Volume failed, no block %i\n", arrPos);
                }
            } // for z
        } // for y
    } // for x
    changed = true;
}


void CubeSet::SetSphere( const glm::vec3 pos, const glm::vec3 radius, const BlockType type, const bool replace ) {
    const float blockWidth = BLOCK_RADIUS*2.0f;
    // Number of blocks per axis
    int blocksX = radius.x/BLOCK_RADIUS;
    int blocksY = radius.y/BLOCK_RADIUS;
    int blocksZ = radius.z/BLOCK_RADIUS;
    
    // Largest radius for ratios
    float largestRadius = radius.x;
    if ( largestRadius < radius.y ) { largestRadius = radius.y; }
    if ( largestRadius < radius.z ) { largestRadius = radius.z; }
    // Ratios along each axis by which to squash the sphere
    float ratioX = largestRadius/radius.x;
    float ratioY = largestRadius/radius.y;
    float ratioZ = largestRadius/radius.z;
    const float epsilon = BLOCK_RADIUS*0.1f;
    glm::vec3 zeroPos = pos-radius+glm::vec3(BLOCK_RADIUS); // Position of block at 0,0,0
    // Add blocks inside squashed radius
    for (int x=0; x < blocksX; x++) {
        for (int y=0; y <= blocksY; y++) {
            for (int z=0; z <= blocksZ; z++) {
                glm::vec3 squash = glm::vec3(ratioX,ratioY,ratioZ);
                glm::vec3 zeroSquash = pos-(radius*squash)+(squash*(float)BLOCK_RADIUS);
                glm::vec3 cubeSquashPos = zeroSquash + glm::vec3(x,y,z)*squash*blockWidth;
                if ( glm::distance(cubeSquashPos, pos) < largestRadius+epsilon ) {
                    glm::vec3 cubePos = zeroPos+(glm::vec3(x,y,z)*blockWidth);
                    int arrPos = PosToArray(cubePos);
                    if ( arrPos != -1 ) {
                        if ( replace || blocks[arrPos].blockType == Type_Empty ) {
                            blocks[arrPos].blockType = type;
                            blocks[arrPos].blockColor = ColorForType(type);
                        }
                    } else {
                        //                        printf("[CubeSet] Sphere failed, no block %i\n", arrPos);
                    }
                }   // if position in sphere
            } // for z
        } // for y
    } // for x
    changed = true;
}

void CubeSet::SetCylinderY( const glm::vec3 pos, const glm::vec3 radius, const BlockType type, const bool replace ) {
    const float blockWidth = BLOCK_RADIUS*2.0f;
    // Number of blocks per axis
    int blocksX = radius.x/BLOCK_RADIUS;
    int blocksY = radius.y/BLOCK_RADIUS;
    int blocksZ = radius.z/BLOCK_RADIUS;
    
    // Largest radius for ratios
    float largestRadius = radius.x;
    if ( largestRadius < radius.z ) { largestRadius = radius.z; }
    // Ratios along each axis by which to squash the sphere
    float ratioX = largestRadius/radius.x;
    float ratioZ = largestRadius/radius.z;
    const float epsilon = BLOCK_RADIUS*0.1f;
    glm::vec3 zeroPos = pos-radius+glm::vec3(BLOCK_RADIUS); // Position of block at 0,0,0
    // Add blocks inside squashed radius
    for (int x=0; x < blocksX; x++) {
        for (int y=0; y <= blocksY; y++) {
            for (int z=0; z <= blocksZ; z++) {
                glm::vec2 xzPos = glm::vec2(pos.x,pos.z);
                glm::vec2 squash = glm::vec2(ratioX,ratioZ);
                glm::vec2 zeroSquash = xzPos-(glm::vec2(radius.x,radius.z)*squash)+(squash*(float)BLOCK_RADIUS);
                glm::vec2 cubeSquashPos = zeroSquash + glm::vec2(x,z)*squash*blockWidth;
                if ( glm::distance(cubeSquashPos, xzPos) < largestRadius+epsilon ) {
                    glm::vec3 cubePos = zeroPos+(glm::vec3(x,y,z)*blockWidth);
                    int arrPos = PosToArray(cubePos);
                    if ( arrPos != -1 ) {
                        if ( replace || blocks[arrPos].blockType == Type_Empty ) {
                            blocks[arrPos].blockType = type;
                            blocks[arrPos].blockColor = ColorForType(type);
                        }
                    } else {
//                        printf("[CubeSet] Cylinder failed, no block %i\n", arrPos);
                    }
                }   // if position in sphere
            } // for z
        } // for y
    } // for x
    changed = true;
}
int CubeSet::Serialise( unsigned char *buffer ) {
    if ( !blocks ) return 0;
    int size = 0;
    // Save 5 byte header and 3 byte version
    memcpy(buffer, CUBESET_HEADER, 5);
    memcpy(buffer+5, CUBESET_VERSION, 3);
    size += 8;
    // Save width and height of object
    size += Serialise::serialise((unsigned int)width_bits, buffer+size);
    size += Serialise::serialise((unsigned int)height_bits, buffer+size);
    // Save number of cubes
    unsigned int numCubes = GetWidth()*GetWidth()*GetHeight();
    size += Serialise::serialise(numCubes, buffer+size);
    // Save cube data
    for (unsigned int i=0; i<numCubes; i++) {
        Block& b = blocks[i];
        size += Serialise::serialise((unsigned int)b.blockType, buffer+size);
        size += Serialise::serialise(b.blockColor.r, buffer+size);
        size += Serialise::serialise(b.blockColor.g, buffer+size);
        size += Serialise::serialise(b.blockColor.b, buffer+size);
        size += Serialise::serialise(b.blockColor.a, buffer+size);
    }
    return size;
}

int CubeSet::Deserialise( const unsigned char *buffer ) {
    int size = 0;
    // Read header and version number
    if ( strncmp( (const char*)buffer, CUBESET_HEADER, 5) == 0 ) {
        size += 5;        // Got a good header, skip forward by 5 bytes
    } else { printf("Object header fail\n"); }
    if ( strncmp( (const char*)buffer+5, CUBESET_VERSION, 3) == 0 ) {
        size += 3;        // Got a good version, skip forward by 3 bytes
    } else { printf("Object version fail\n"); }
    // Load width and height of object
    width_bits = Serialise::deserialiseInt(buffer+size);
    size += SERIALISED_INT_SIZE;
    height_bits = Serialise::deserialiseInt(buffer+size);
    size += SERIALISED_INT_SIZE;
    // Load number of cubes
    unsigned int numCubes = Serialise::deserialiseInt(buffer+size);
    size += SERIALISED_INT_SIZE;
    // Prepare block storage
    if ( blocks ) { delete [] blocks; blocks = NULL; }
    blocks = new Block[numCubes];
    // Load block data
    for (unsigned int i=0; i<numCubes; i++) {
        Block& b = blocks[i];
        b.blockType = (BlockType)Serialise::deserialiseInt(buffer+size);
        size += SERIALISED_INT_SIZE;
        b.blockColor.r = Serialise::deserialiseFloat(buffer+size);
        size += SERIALISED_FLOAT_SIZE;
        b.blockColor.g = Serialise::deserialiseFloat(buffer+size);
        size += SERIALISED_FLOAT_SIZE;
        b.blockColor.b = Serialise::deserialiseFloat(buffer+size);
        size += SERIALISED_FLOAT_SIZE;
        b.blockColor.a = Serialise::deserialiseFloat(buffer+size);
        size += SERIALISED_FLOAT_SIZE;
    }
    changed = true;
    return size;
}
