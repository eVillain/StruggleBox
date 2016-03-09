#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

/// Mother of all randoms!
/// Contains (pseudo)random number generators
class Random {
public:
    static uint32_t RandomBits();
    static double RandomDouble();
    static int RandomInt(int min, int max);
    static void RandomSeed (int seed);
};

#endif /* RANDOM_H */
