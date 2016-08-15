#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H


#include "GFXDefines.h"
#include <map>
#include <string>

class Texture;

// Texture Manager class
// Uses LibPNG to load .png files
// For use with OpenGL and the LibPNG library
class TextureManager
{
public:
    static TextureManager* Inst();
    static void Destroy();
    
//    void SetPath( std::string path ) { filePath = path; };

    // Load a texture an make it the current texture
    Texture* LoadTexture( const std::string filePath,
                         const std::string fileName,       // Texture file name, will serve as reference
                         GLint wrapMethod = GL_REPEAT,      // Texture wrapping method
                         GLint filter = GL_NEAREST,         // Texture filtering (mag and min for now)
                         GLint level = 0,                   // Texture mipmapping level
                         GLint border = 0);                 // Texture border size
    
    Texture* GetTexture( const std::string fileName );
    Texture* GetTextureForID( const int serialID );
    std::string GetName( const Texture* tex );
    std::string GetNameForID( const int serialID );
    int GetSerialisedID( const Texture* tex );
    int GetSerialisedID( const std::string fileName );

    // Free the memory for a texture
    bool UnloadTexture( const std::string fileName );
    bool UnloadTexture( Texture* tex );

    // Set the current texture
    bool BindTexture( const std::string fileName );

    // Free all texture memory
    void UnloadAllTextures( void );
    void UnloadTextures( void );
    
    // Reload all the textures
    void ReloadTextures ( void );
    
    unsigned int Serialise( unsigned char* buffer );
    unsigned int Deserialise( const unsigned char* data );
    unsigned int GetSerialisedLength();
    
protected:
    TextureManager();
    ~TextureManager();

    static TextureManager* m_inst;                              // Texture manager instance pointer
    std::map<std::string, Texture*> textureMap;                 // Map of loaded textures, by name
//    std::string filePath;                                       // Path to default texture folder
};

#endif