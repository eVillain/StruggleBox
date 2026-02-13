#pragma once

#include <climits>
#include <type_traits>

namespace MathUtils
{
    static const long double PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L;

    // Function to convert degrees to radians
    //const float TO_RADS = 3.141592654f / 180.0f; // The value of 1 degree in radians
    inline static float ToRadians(const float& theAngleInDegrees)
    {
        return theAngleInDegrees * 3.141592654f / 180.0f;
    }
    
    // Linear interpolate floats
    inline static float Lerp(float v0,
                             float v1,
                             float t)
    {
        return v0+(v1-v0)*t;
    }
    
    inline static float Lerp(float x,
                             float x1,
                             float x2,
                             float q00,
                             float q01)
    {
        return ((x2 - x) / (x2 - x1)) * q00 + ((x - x1) / (x2 - x1)) * q01;
    }
    
    inline static float BiLerp(float x,
                               float y,
                               float q11,
                               float q12,
                               float q21,
                               float q22,
                               float x1,
                               float x2,
                               float y1,
                               float y2) {
        float r1 = Lerp(x, x1, x2, q11, q21);
        float r2 = Lerp(x, x1, x2, q12, q22);
        
        return Lerp(y, y1, y2, r1, r2);
    }
    
    inline static float TriLerp(float x,
                                float y,
                                float z,
                                float q000,
                                float q001,
                                float q010,
                                float q011,
                                float q100,
                                float q101,
                                float q110,
                                float q111,
                                float x1,
                                float x2,
                                float y1,
                                float y2,
                                float z1,
                                float z2) {
        float x00 = Lerp(x, x1, x2, q000, q100);
        float x10 = Lerp(x, x1, x2, q010, q110);
        float x01 = Lerp(x, x1, x2, q001, q101);
        float x11 = Lerp(x, x1, x2, q011, q111);
        float r0 = Lerp(y, y1, y2, x00, x01);
        float r1 = Lerp(y, y1, y2, x10, x11);
        
        return Lerp(z, z1, z2, r0, r1);
    }
    
    // Relatively quick number clamps
    template <typename T>
    inline static T Clamp(const T x, const T a, const T b)
    {
        return x < a ? a : (x > b ? b : x);
    }
    
    // Bigger of two values
    template <typename T>
    static inline T Max(const T a, const T b)
    {
        return (a > b ? a : b);
    }

    // Smaller of two values
    template <typename T>
    static inline T Min(const T a, const T b)
    {
        return (a > b ? b : a);
    }

    template <typename UnsignedType>
    UnsignedType round_up_to_power_of_2(UnsignedType v) {
        static_assert(std::is_unsigned<UnsignedType>::value, "Only works for unsigned types");
        v--;
        for (size_t i = 1; i < sizeof(v) * CHAR_BIT; i *= 2) //Prefer size_t "Warning comparison between signed and unsigned integer"
        {
            v |= v >> i;
        }
        return ++v;
    }
}
