#ifndef SERIALISE_H
#define SERIALISE_H

#include <string>
#include <glm/glm.hpp>

#define SERIALISED_FLOAT_SIZE 4
#define SERIALISED_DOUBLE_SIZE 8
#define SERIALISED_BOOL_SIZE 1
#define SERIALISED_INT_SIZE 4
#define SERIALISED_SHORT_SIZE 2
#define SERIALISED_LONG_SIZE 4

#define SERIALISED_VEC3_SIZE 12
#define SERIALISED_VEC4_SIZE 16
#define SERIALISED_COLOR_SIZE 16

/**
 * Static methods for serialising and deserialising primitive data types.
 */
namespace Serialise {

    /**
     * Turns a short (2 bytes) into 2 chars.  Supplied output parameter must
     * point to allocated memory at least 2 bytes long.
     * @param value Short to serialise.
     * @param output Char array in which to store serialised short.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(unsigned short value, unsigned char* output);
    /**
     * Extracts a short (2 bytes) from the supplied char array.
     * @param data Data to extract short from.
     * @return The deserialised short.
     */
    unsigned short deserialiseShort(const unsigned char* data);
    
    /**
     * Turns a long (4 bytes) into 4 chars.  Supplied output parameter must
     * point to allocated memory at least 4 bytes long.
     * @param value Long to serialise.
     * @param output Char array in which to store serialised short.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(unsigned long value, unsigned char* output);
    /**
     * Extracts a long (4 bytes) from the supplied char array.
     * @param data Data to extract short from.
     * @return The deserialised short.
     */
    unsigned long deserialiseLong(const unsigned char* data);
    
    
    /**
     * Turns an int (4 bytes) into 4 chars.  Supplied output parameter must
     * point to allocated memory at least 4 bytes long.  Integer is stored
     * in big endian order.
     * @param value Integer to serialise.
     * @param output Char array in which to store serialised int.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(unsigned int value, unsigned char* output);
    /**
     * Extracts an int (4 bytes) from the supplied char array.
     * @param data Data to extract int from.
     * @return The deserialised int.
     */
    unsigned int deserialiseInt(const unsigned char* data);
    
    /**
     * Turns a bool (1 bit) into 1 chars.  Supplied output parameter must
     * point to allocated memory at least 1 byte long.
     * @param value Bool to serialise.
     * @param output Char array in which to store serialised bool.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(bool value, unsigned char* output);
    /**
     * Extracts a bool (1 bit) from the supplied char array.
     * @param data Data to extract bool from.
     * @return The deserialised bool.
     */
    bool deserialiseBool(const unsigned char* data);

    /**
     * Turns a double (8 bytes) into 8 chars.  Supplied output parameter must
     * point to allocated memory at least 8 bytes long.
     * @param value Double to serialise.
     * @param output Char array in which to store serialised double.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(double value, unsigned char* output);
    /**
     * Extracts a double (8 bytes) from the supplied char array.
     * @param data Data to extract double from.
     * @return The deserialised double.
     */
    double deserialiseDouble(const unsigned char* data);
    
    /**
     * Turns a float (4 bytes) into 4 chars.  Supplied output parameter must
     * point to allocated memory at least 4 bytes long.
     * @param value Float to serialise.
     * @param output Char array in which to store serialised float.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(float value, unsigned char* output);
    /**
     * Extracts a float (4 bytes) from the supplied char array.
     * @param data Data to extract float from.
     * @return The deserialised float.
     */
    float deserialiseFloat(const unsigned char* data);
    
    /**
     * Turns a glm::vec3 struct (12 bytes) into 12 chars.  Supplied output
     * parameter must point to allocated memory at least 12 bytes long.
     * @param vector glm::vec3 to serialise.
     * @param output Char array in which to store serialised cpVect.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(const glm::vec3& vector, unsigned char* output);
    /**
     * Extracts a glm::vec3 (12 bytes) from the supplied char array.
     * @param data Data to extract glm::vec3 from.
     * @return The deserialised cpVect.
     */
    glm::vec3 deserialiseVec3(const unsigned char* data);
    
    /**
     * Turns a glm::vec4 struct (16 bytes) into 16 chars.  Supplied output
     * parameter must point to allocated memory at least 16 bytes long.
     * @param vector glm::vec4 to serialise.
     * @param output Char array in which to store serialised cpVect.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(const glm::vec4& vector, unsigned char* output);
    /**
     * Extracts a glm::vec4 (16 bytes) from the supplied char array.
     * @param data Data to extract glm::vec3 from.
     * @return The deserialised cpVect.
     */
    glm::vec4 deserialiseVec4(const unsigned char* data);
    
    /**
     * Turns a Color struct (16 bytes) into 16 chars.  Supplied output
     * parameter must point to allocated memory at least 16 bytes long.
     * @param Color struct to serialise.
     * @param output Char array in which to store serialised cpVect.
     * @return Number of bytes stored in the output buffer.
     */
//    unsigned int serialise(const Color& vector, unsigned char* output);
    /**
     * Extracts a glm::vec4 (16 bytes) from the supplied char array.
     * @param data Data to extract glm::vec3 from.
     * @return The deserialised cpVect.
     */
//    glm::vec4 deserialiseColor(const unsigned char* data);
    
    /**
     * Turns an std::string into chars. Supplied output parameter must
     * point to allocated memory at least string.length() bytes long.
     * @param value string to serialise.
     * @param output Char array in which to store serialised bool.
     * @return Number of bytes stored in the output buffer.
     */
    unsigned int serialise(std::string value, unsigned char* output);
    /**
     * Extracts an std::string from the supplied char array.
     * @param data Data to extract string from.
     * @return The deserialised string.
     */
    std::string deserialiseString(const unsigned char* data);
    
    /**
     * Packs a float or double into IEEE-754 format.
     * Taken from http://beej.us/guide/bgnet/output/html/multipage/advanced.html#serialization
     * @param f Value to pack.
     * @param bits Number of bits in the value.
     * @param expbits Number of bits in the exponent.
     */
    long long pack754(long double f, unsigned bits, unsigned expbits);
    /**
     * Unpacks an IEEE-754 value into a float or double.
     * Taken from http://beej.us/guide/bgnet/output/html/multipage/advanced.html#serialization
     * @param i Value to unpack.
     * @param bits Number of bits in the value.
     * @param expbits Number of bits in the exponent.
     */
    long double unpack754(long long i, unsigned bits, unsigned expbits);
};

#endif /* SERIALIZE_H */
