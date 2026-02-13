#include "Random.h"

uint32_t rand_x[5]; // Mother of all randoms history buffer

// Output random bits
uint32_t Random::RandomBits()
{
    uint64_t sum;
    sum = (uint64_t)2111111111UL * (uint64_t)rand_x[3] +
    (uint64_t)1492 * (uint64_t)(rand_x[2]) +
    (uint64_t)1776 * (uint64_t)(rand_x[1]) +
    (uint64_t)5115 * (uint64_t)(rand_x[0]) +
    (uint64_t)rand_x[4];
    rand_x[3] = rand_x[2];  rand_x[2] = rand_x[1];  rand_x[1] = rand_x[0];
    rand_x[4] = (uint32_t)(sum >> 32);                  // Carry
    rand_x[0] = (uint32_t)sum;                          // Low 32 bits of sum
    return rand_x[0];
}

// returns a random number between 0 and 1:
double Random::RandomDouble()
{
    return (double)RandomBits() * (1./(65536.*65536.));
}

// returns integer random number in desired interval:
int Random::RandomInt(int min, int max)
{
    // Output random integer in the interval min <= x <= max
    // Relative error on frequencies < 2^-32
    if (max <= min)
    {
        if (max == min) return min; else return 0x80000000;
    }
    // Assume 64 bit integers supported. Use multiply and shift method
    uint32_t interval;                  // Length of interval
    uint64_t longRand;                   // Random bits * interval
    uint32_t intRand;                      // Longran / 2^32
    
    interval = (uint32_t)(max - min + 1);
    longRand  = (uint64_t)RandomBits() * interval;
    intRand = (uint32_t)(longRand >> 32);
    // Convert back to signed and return result
    return (int32_t)intRand + min;
}

// this function initializes the random number generator:
void Random::RandomSeed (int seed)
{
    int i;
    uint32_t s = seed;
    // make random numbers and put them into the buffer
    for (i = 0; i < 5; i++)
    {
        s = s * 29943829 - 1;
        rand_x[i] = s;
    }
    // randomize some more
    for (i=0; i<19; i++) RandomBits();
}
