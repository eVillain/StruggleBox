//
//  Random.h
//  DungeonSmith
//
//  Created by The Drudgerist on 20/04/15.
//  Copyright (c) 2015 The Drudgerist. All rights reserved.
//
//========================================================================
// Random
// Mother of all randoms!
// (Pseudo)Random number generators
//========================================================================

#ifndef __DungeonSmith__Random__
#define __DungeonSmith__Random__

#include <stdint.h>

class Random {
public:
    static uint32_t RandomBits();
    static double RandomDouble();
    static int RandomInt(int min, int max);
    static void RandomSeed (int seed);
};


#endif /* defined(__DungeonSmith__Random__) */
