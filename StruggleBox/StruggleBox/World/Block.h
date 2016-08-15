//
//  Block.h
//  Bloxelizer
//
//  Created by The Drudgerist on 7/31/13.
//
//

#ifndef BWO_BLOCK_H
#define BWO_BLOCK_H

#include "BlockDefs.h"

class Block {
public:
    Color blockColor;
    BlockType blockType;
    static int totalBlocks;

    Block( const Color& c=COLOR_NONE, const BlockType t=Type_Empty ):
    blockColor(c), blockType(t) { totalBlocks++; };
    ~Block() { totalBlocks--; };
};

#endif
