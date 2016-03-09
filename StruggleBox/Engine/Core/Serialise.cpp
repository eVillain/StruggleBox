#include "Serialise.h"

namespace Serialise {
    // Shorts
    unsigned int serialise(unsigned short value, unsigned char* output) {
        *output = (char)(value >> 8);
        *(output + 1) = (char)value;
        return SERIALISED_SHORT_SIZE;
    }
    unsigned short deserialiseShort(const unsigned char* data) {
        unsigned short output;
        output = *data << 8;
        output |= *(data + 1);
        return output;
    }
    
    // Longs
    unsigned int serialise(unsigned long value, unsigned char* output) {
        *output = (char)(value >> 24);
        *(output + 1) = (char)(value >> 16);
        *(output + 2) = (char)(value >> 8);
        *(output + 3) = (char)value;
        return SERIALISED_LONG_SIZE;
    }
    unsigned long deserialiseLong(const unsigned char* data) {
        unsigned long output;
        output = *data << 24;
        output |= *(data + 1) << 16;
        output |= *(data + 2) << 8;
        output |= *(data + 3);
        return output;
    }
    
    // Integers
    unsigned int serialise(unsigned int value, unsigned char* output) {
        *output = (char)(value >> 24);
        *(output + 1) = (char)(value >> 16);
        *(output + 2) = (char)(value >> 8);
        *(output + 3) = (char)value;	
        return SERIALISED_INT_SIZE;
    }
    unsigned int deserialiseInt(const unsigned char* data) {
        unsigned int output;
        output = *data << 24;
        output |= *(data + 1) << 16;
        output |= *(data + 2) << 8;
        output |= *(data + 3);
        return output;
    }
    
    // Booleans
    unsigned int serialise(bool value, unsigned char* output) {
        *output = (bool)value;
        return SERIALISED_BOOL_SIZE;
    }
    bool deserialiseBool(const unsigned char* data) {
        bool output;
        // output = *data;
        output = *(bool*)data; // fixed win compiler warning, might be iffy
        return output;
    }
    
    // Doubles
    unsigned int serialise(double value, unsigned char* output) {
        // Get a packed version of the double
        unsigned long long int packed = pack754(value, 64, 11);
        *output = (char)(packed >> 56);
        *(output + 1) = (char)(packed >> 48);
        *(output + 2) = (char)(packed >> 40);
        *(output + 3) = (char)(packed >> 32);
        *(output + 4) = (char)(packed >> 24);
        *(output + 5) = (char)(packed >> 16);
        *(output + 6) = (char)(packed >> 8);
        *(output + 7) = (char)packed;
        return SERIALISED_DOUBLE_SIZE;
    }
    double deserialiseDouble(const unsigned char* data) {
        // Restore the serialised double to a packed 64-bit int
        unsigned long long int packed;
        packed = ((unsigned long long int)((*data) & 0xFF)) << 56;
        packed |= ((unsigned long long int)(*(data + 1) & 0xFF)) << 48;
        packed |= ((unsigned long long int)(*(data + 2) & 0xFF)) << 40;
        packed |= ((unsigned long long int)(*(data + 3) & 0xFF)) << 32;
        packed |= ((unsigned long long int)(*(data + 4) & 0xFF)) << 24;
        packed |= ((unsigned long long int)(*(data + 5) & 0xFF)) << 16;
        packed |= ((unsigned long long int)(*(data + 6) & 0xFF)) << 8;
        packed |= ((unsigned long long int)(*(data + 7) & 0xFF));
        // Get an unpacked version of the double
        double unpacked = unpack754(packed, 64, 11);
        return unpacked;
    }
    
    // Floats
    unsigned int serialise(float value, unsigned char* output) {
        // Get a packed version of the float
        unsigned int packed = (unsigned int)pack754(value, 32, 8);
        *output = (char)(packed >> 24);
        *(output + 1) = (char)(packed >> 16);
        *(output + 2) = (char)(packed >> 8);
        *(output + 3) = (char)packed;
        return SERIALISED_FLOAT_SIZE;
    }
    float deserialiseFloat(const unsigned char* data) {
        // Restore the serialised float to a packed int
        unsigned int packed;
        packed = ((unsigned int)((*data) & 0xFF)) << 24;
        packed |= ((unsigned int)(*(data + 1) & 0xFF)) << 16;
        packed |= ((unsigned int)(*(data + 2) & 0xFF)) << 8;
        packed |= ((unsigned int)(*(data + 3) & 0xFF));
        // Get an unpacked version of the float
        float unpacked = (float)unpack754(packed, 32, 8); // (float) fixed win compiler warning, might be iffy
        return unpacked;
    }
    
    // Vector (3 components)
    unsigned int serialise(const glm::vec3 &vector, unsigned char* output) {
        output += serialise(vector.x, output);
        output += serialise(vector.y, output);
        output += serialise(vector.z, output);
        return SERIALISED_VEC3_SIZE;
    }
    glm::vec3 deserialiseVec3(const unsigned char* data) {
        float x = deserialiseFloat(data);
        float y = deserialiseFloat(data + 4);
        float z = deserialiseFloat(data + 8);
        return glm::vec3(x,y,z);
    }
    
    // Vector (4 components)
    unsigned int serialise(const glm::vec4 &vector, unsigned char* output) {
        output += serialise(vector.x, output);
        output += serialise(vector.y, output);
        output += serialise(vector.z, output);
        output += serialise(vector.w, output);
        return SERIALISED_VEC4_SIZE;
    }
    glm::vec4 deserialiseVec4(const unsigned char* data) {
        float x = deserialiseFloat(data);
        float y = deserialiseFloat(data + 4);
        float z = deserialiseFloat(data + 8);
        float w = deserialiseFloat(data + 12);
        return glm::vec4(x,y,z,w);
    }
    
    // Strings
    unsigned int serialise(std::string value, unsigned char* output) {
        unsigned int length = (int)value.length();
        unsigned int size = serialise(length, output);
        if ( length > 0 ) {
            memcpy(output+size, value.c_str(), value.length());
            size += length;
        }
        return size;
    }
    std::string deserialiseString(const unsigned char* data) {
        unsigned int length = deserialiseInt( data );
        size_t size = length;
        unsigned int dataPos = 4;
        if ( length > 0 ) {
            std::string output((char*)(data+dataPos), size);
            return output;
        }
        return "";
    }
    
    // Packing and unpacking functions
    long long pack754(long double f, unsigned bits, unsigned expbits) {
        long double fnorm;
        int shift;
        long long sign, exp, significand;
        unsigned significandbits = bits - expbits - 1; // -1 for sign bit
        
        if (f == 0.0) return 0; // get this special case out of the way
        
        // (long long) fixed win compiler warning, might be iffy
        if (f == INFINITY) return (long long)INFINITY;	// Special case for infinity
        
        // check sign and begin normalization
        if (f < 0) { sign = 1; fnorm = -f; }
        else { sign = 0; fnorm = f; }
        
        // get the normalized form of f and track the exponent
        shift = 0;
        while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
        while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
        fnorm = fnorm - 1.0;
        
        // calculate the binary form (non-float) of the significand data
        // (long long) fixed win compiler warning, might be iffy
        significand = (long long)(fnorm * ((1LL<<significandbits) + 0.5f));
        
        // get the biased exponent
        exp = shift + ((1<<(expbits-1)) - 1); // shift + bias
        
        // return the final answer
        return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
    }
    long double unpack754(long long i, unsigned bits, unsigned expbits) {
        long double result;
        long long shift;
        unsigned bias;
        unsigned significandbits = bits - expbits - 1; // -1 for sign bit
        
        if (i == 0) return 0.0;
        
        if (i == INFINITY) return INFINITY;	// Special case for infinity
        
        // pull the significand
        result = (long double)(i&((1LL<<significandbits)-1)); // mask
        result /= (1LL<<significandbits); // convert back to float
        result += 1.0f; // add the one back on
        
        // deal with the exponent
        bias = (1<<(expbits-1)) - 1;
        shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
        while(shift > 0) { result *= 2.0; shift--; }
        while(shift < 0) { result /= 2.0; shift++; }
        
        // sign it
        result *= (i>>(bits-1))&1? -1.0: 1.0;
        
        return result;
    }
    
    

};
