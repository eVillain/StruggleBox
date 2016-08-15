#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include "GFXDefines.h"

class Texture
{
public:
	// TODO: REMOVE THIS SHIT ASAP AND REPLACE WITH SHARED_PTR!!!
    unsigned int refCount;  // How many sprites are using texture

    Texture( std::string fileName,
            GLint wrap = GL_REPEAT,
            GLint minF = GL_NEAREST,
            GLint magF = GL_NEAREST,
            GLint level = 0);
    ~Texture();
    
    void LoadFromFile( const std::string fileName );
    void LoadFromPNG( const std::string fileName );
    void LoadFromPNGData( const char* data );

    void Bind( void ) const;
    void Unload( void );
    
    void DrawAt(int posX, int posY);
    
    bool IsLoaded( void ) const { return loaded; };
    GLuint GetID( void ) const { return textureID; };
    GLuint GetWidth( void ) const { return width; };
    GLuint GetHeight( void ) const { return height; };
    std::string GetPath( void ) const { return filePath; };

private:
	bool loaded;            // Whether texture is loaded and ready
	GLuint textureID;       // Texture ID Used To Select A Texture
	GLint width;            // Image Width
	GLint height;           // Image Height
	GLint format;           // Image Type (GL_RGB, GL_RGBA)
	GLint bpp;              // Image Color Depth In Bits Per Pixel

	GLint wrapMethod;       // Texture wrapping method
	GLint minFilter;        // Texture minimization filtering
	GLint magFilter;        // Texture magnification filtering
	GLint mipLevel;         // Texture mipmapping level
	std::string filePath;   // Path to folder containing texture
};




#endif
