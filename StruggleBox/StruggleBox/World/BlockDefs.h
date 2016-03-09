//
//  BlockDefs.h
//  Bloxelizer
//
//  Created by The Drudgerist on 7/18/13.
//
//

#ifndef BWO_BLOCK_DEFS_H
#define BWO_BLOCK_DEFS_H

#include <string>
#include "GFXDefines.h"

#define BLOCK_HEADER "BWBlk"
#define BLOCK_RADIUS 0.25

typedef unsigned char SmitherType;


// Types of blocks, will be used later for distinguishing between block behaviors
enum BlockType {
    // Transparent types
    Type_Empty = 0,         // Contains nothing
    // Semi-transparent types
    Type_Water = 1,
    Type_Blood = 2,
    Type_Mud = 3,
    Type_Ice = 4,
    Type_Glass = 5,
    Type_Diamond = 6,
    Type_Ruby = 7,
    Type_Emerald = 8,
    Type_Sapphire = 9,
    // Terrain types
    Type_Dirt = 16,
    Type_Grass = 17,
    Type_Snow = 18,
    Type_Sand = 19,
    Type_Gravel = 20,
    Type_Rock = 21,
    Type_Wood_1 = 22,
    Type_Wood_2 = 23,
    Type_Wood_3 = 24,
    Type_Wood_Jungle = 25,
    Type_Leaves_1 = 26,
    Type_Leaves_2 = 27,
    Type_Leaves_3 = 28,
    Type_Leaves_Jungle = 29,
    
    // Materials
    Type_Leather_Light = 40,
    Type_Leather_Heavy = 41,
    Type_Hide_Light = 42,
    Type_Hide_Heavy = 43,
    Type_Skin = 44,
    Type_Bone = 45,
    Type_Meat = 46,
    Type_Brain = 47,
    Type_Coal = 48,
    Type_Bronze = 49,
    Type_Iron = 50,
    Type_Steel = 51,
    Type_Gold = 52,
    Type_Silver = 53,

    // Color types for debugging
    Type_Red = 65,
    Type_Green = 66,
    Type_Blue = 67,
    Type_Yellow = 68,
    Type_Cyan = 69,
    Type_Purple = 70,
    
};
inline static std::string NameForType( BlockType type ) {
    switch (type) {
        case Type_Empty: { return "Empty Block"; } break;
        case Type_Blood: { return "Blood Block"; } break;
        case Type_Water: { return "Water Block"; } break;
        case Type_Ice: { return "Ice Block"; } break;
        case Type_Mud: { return "Mud Block"; } break;
        case Type_Glass: { return "Glass Block"; } break;
        case Type_Diamond: { return "Diamond Block"; } break;
        case Type_Ruby: { return "Ruby Block"; } break;
        case Type_Emerald: { return "Emerald Block"; } break;
        case Type_Sapphire: { return "Sapphire Block"; } break;

        case Type_Leaves_1: { return "Leaves1"; } break;
        case Type_Leaves_2: { return "Leaves2"; } break;
        case Type_Leaves_3: { return "Leaves3"; } break;
        case Type_Leaves_Jungle: { return "Jungle Leaves"; } break;

        case Type_Dirt: { return "Dirt Block"; } break;
        case Type_Snow: { return "Snow Block"; } break;
        case Type_Grass: { return "Grass Block"; } break;
        case Type_Sand: { return "Sand Block"; } break;
        case Type_Gravel: { return "Gravel Block"; } break;
        case Type_Rock: { return "Stone Block"; } break;
        case Type_Wood_1: { return "Oak Tree Block"; } break;
        case Type_Wood_2: { return "Tree 2 Block"; } break;
        case Type_Wood_3: { return "Tree 3 Block"; } break;
        case Type_Wood_Jungle: { return "Jungle Tree Block"; } break;

        case Type_Leather_Light: { return "Leather(L) Block"; } break;
        case Type_Leather_Heavy: { return "Leather(H) Block"; } break;
        case Type_Hide_Light: { return "Hide(L) Block"; } break;
        case Type_Hide_Heavy: { return "Hide(H) Block"; } break;
        case Type_Bone: { return "Bone Block"; } break;
        case Type_Skin: { return "Skin Block"; } break;
        case Type_Meat: { return "Meat Block"; } break;
        case Type_Brain: { return "Brainmatter Block"; } break;
        case Type_Coal: { return "Coal Block"; } break;
        case Type_Bronze: { return "Bronze Block"; } break;
        case Type_Iron: { return "Iron Block"; } break;
        case Type_Steel: { return "Steel Block"; } break;
        case Type_Gold: { return "Gold Block"; } break;
        case Type_Silver: { return "Silver Block"; } break;

        case Type_Red: { return "Red Block"; } break;
        case Type_Green: { return "Green Block"; } break;
        case Type_Blue: { return "Blue Block"; } break;
        case Type_Yellow: { return "Yellow Block"; } break;
        case Type_Cyan: { return "Cyan Block"; } break;
        case Type_Purple: { return "Purple Block"; } break;
    
        default: { return "Unknown Type"; } break;
    }
}
inline static Color ColorForType( BlockType type ) {
    switch (type) {
        case Type_Empty: { return COLOR_NONE; } break;
        case Type_Water: { return RGBAByteColor(91, 110, 225, 127); } break;
        case Type_Ice: { return RGBAByteColor(203,219,252,127); } break;
        case Type_Mud: { return RGBAByteColor(138, 111, 48, 191); } break;
        case Type_Blood: { return RGBAByteColor(172, 50, 50, 191); } break;
        case Type_Glass: { return RGBAColor(1.0f, 1.0f, 1.0f, 0.25f); } break;
        case Type_Diamond: { return RGBAColor(1.0f, 1.0f, 1.0f, 0.1f); } break;
        case Type_Ruby: { return RGBAByteColor(217, 87, 99, 127); } break;
        case Type_Emerald: { return RGBAByteColor(153, 229, 80, 127); } break;
        case Type_Sapphire: { return RGBAByteColor(99, 155, 255, 127); } break;

        case Type_Dirt: { return RGBAByteColor(143, 86, 59, 255); } break;
        case Type_Snow: { return RGBAByteColor(255, 255, 255, 255); } break;
        case Type_Grass: { return RGBAByteColor(106, 190, 48, 255); } break;
        case Type_Sand: { return RGBAByteColor(251, 242, 54, 255); } break;
        case Type_Gravel: { return RGBAByteColor(132, 126, 135, 255); } break;
        case Type_Rock: { return RGBAByteColor(105, 106, 106, 255); } break;
        case Type_Wood_1: { return RGBAByteColor(102, 57, 49, 255); } break;
        case Type_Wood_2: { return RGBAByteColor(217, 160, 102, 255); } break;
        case Type_Wood_Jungle: { return RGBAByteColor(138, 111, 48, 255); } break;
        case Type_Leaves_1: { return RGBAByteColor(153, 229, 80, 255); } break;
        case Type_Leaves_2: { return RGBAByteColor(75, 105, 47, 255); } break;
        case Type_Leaves_Jungle: { return RGBAByteColor(143, 151, 74, 255); } break;

        case Type_Leather_Light: { return RGBAColor(0.515f, 0.445f, 0.305f, 1.0f); } break;
        case Type_Leather_Heavy: { return RGBAColor(0.270f, 0.230f, 0.130f, 1.0f); } break;
        case Type_Hide_Light: { return RGBAColor(0.615f, 0.545f, 0.405f, 1.0f); } break;
        case Type_Hide_Heavy: { return RGBAColor(0.225f, 0.210f, 0.105f, 1.0f); } break;

        case Type_Bone: { return RGBAColor(0.85f, 0.85f, 0.9f, 1.0f); } break;
 
        case Type_Skin: { return RGBAByteColor(238, 195, 154, 255); } break;
        case Type_Meat: { return RGBAByteColor(217, 87, 99, 255); } break;
        case Type_Brain: { return RGBAColor(1.0f, 0.85f, 0.85f, 1.0f); } break;
        case Type_Bronze: { return RGBAColor(1.0f, 0.65f, 0.45f, 1.0f); } break;
        case Type_Iron: { return RGBAColor(0.45f, 0.45f, 0.6f, 1.0f); } break;
        case Type_Steel: { return RGBAColor(0.6f, 0.6f, 0.67f, 1.0f); } break;
        case Type_Gold: { return RGBAColor(0.85f, 0.85f, 0.4f, 1.0f); } break;
        case Type_Silver: { return RGBAColor(0.85f, 0.85f, 0.95f, 1.0f); } break;

        
        case Type_Red: { return RGBAColor(1.0f, 0.0f, 0.0f, 1.0f); } break;
        case Type_Green: { return RGBAColor(0.0f, 1.0f, 0.0f, 1.0f); } break;
        case Type_Blue: { return RGBAColor(0.0f, 0.0f, 1.0f, 1.0f); } break;

        case Type_Yellow: { return RGBAColor(1.0f, !.0f, 0.0f, 1.0f); } break;
        case Type_Cyan: { return RGBAColor(0.0f, 1.0f, 1.0f, 1.0f); } break;
        case Type_Purple: { return RGBAColor(1.0f, 0.0f, 1.0f, 1.0f); } break;
            
        default: { return COLOR_PURPLE; break; }
    }
}
inline static float SpecularForType( BlockType type ) {
    switch (type) {
        case Type_Empty: { return 0.0f; } break;
        case Type_Water: { return 0.75f; } break;
        case Type_Blood: { return 0.75f; } break;
        case Type_Mud: { return 0.75f; } break;
        case Type_Ice: { return 0.7f; } break;
        case Type_Glass: { return 0.5f; } break;
        case Type_Diamond: { return 1.0f; } break;
        case Type_Ruby: { return 0.8f; } break;
        case Type_Emerald: { return 0.8f; } break;
        case Type_Sapphire: { return 0.8f; } break;

            
        case Type_Dirt: { return 0.0f; } break;
        case Type_Snow: { return 1.0f; } break;
        case Type_Grass: { return 0.0f; } break;
        case Type_Sand: { return 0.0f; } break;
        case Type_Gravel: { return 0.0f; } break;
        case Type_Rock: { return 0.0f; } break;
        case Type_Wood_1: { return 0.0f; } break;
        case Type_Wood_2: { return 0.0f; } break;
        case Type_Wood_3: { return 0.0f; } break;
        case Type_Wood_Jungle: { return 0.0f; } break;
        case Type_Leaves_1: { return 0.3f; } break;
        case Type_Leaves_2: { return 0.3f; } break;
        case Type_Leaves_3: { return 0.3f; } break;
        case Type_Leaves_Jungle: { return 0.3f; } break;

        case Type_Bone: { return 1.0f; } break;
        case Type_Skin: { return 0.1f; } break;
        case Type_Meat: { return 1.0f; } break;
        case Type_Brain: { return 1.0f; } break;
        case Type_Bronze: { return 1.0f; } break;
        case Type_Iron: { return 1.0f; } break;
        case Type_Steel: { return 1.0f; } break;
        case Type_Gold: { return 1.0f; } break;
        case Type_Silver: { return 1.0f; } break;

        case Type_Red: { return 0.0f; } break;
        case Type_Green: { return 0.0f; } break;
        case Type_Blue: { return 0.0f; } break;
            
        case Type_Yellow: { return 0.0f; } break;
        case Type_Cyan: { return 0.0f; } break;
        case Type_Purple: { return 0.0f; } break;
            
        default: { return 0.0f; break; }
    }
}
#endif
