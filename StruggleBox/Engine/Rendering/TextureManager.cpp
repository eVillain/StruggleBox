//**********************************************
//Singleton Texture Manager class
//For use with OpenGL and the LibPNG library
//**********************************************

#include "TextureManager.h"
#include "Texture.h"
#include "SysCore.h"
#include "Log.h"

TextureManager* TextureManager::m_inst(0);

TextureManager* TextureManager::Inst() {
    if(m_inst==0) {
        m_inst = new TextureManager();
        atexit(Destroy);
    }
    return m_inst;
}

void TextureManager::Destroy() {
    if ( m_inst != 0 ) {
        m_inst->UnloadAllTextures();
        delete m_inst;
        m_inst = 0 ;
    }
}

TextureManager::TextureManager()
{ }

TextureManager::~TextureManager()
{
    //UnloadAllTextures();
    //delete m_inst;
    //m_inst = NULL;
}
Texture* TextureManager::LoadTexture( const std::string filePath,
                                     const std::string fileName,
                                     GLint wrapMethod,GLint filter,
                                     GLint level, GLint border )
{
    // If a texture with this name is already loaded just return the previously loaded one
    if( textureMap.find(fileName) != textureMap.end() ) {
        Log::Debug("[TextureManager] Tried to load texture %s which was already loaded...", fileName.c_str() );
        return textureMap[fileName];
    }
     // Take the directory path for textures and append the filename
	std::string fullPath = filePath;
	fullPath.append( fileName );
    
    Texture* newTex = new Texture( fullPath, wrapMethod, filter, filter, level );
    if ( newTex->IsLoaded() ) {
        // Store the texture ID mapping
        textureMap[fileName] = newTex;
    } else {
        // Texture loading failed, delete it
        delete newTex;
        return NULL;
    }
    // Return success
    return newTex;
}
Texture* TextureManager::GetTexture( const std::string fileName )  {
    // If a texture with this name is already loaded just return it
    if( textureMap.find(fileName) != textureMap.end() ) {
        return textureMap[fileName];
    } else {
        Log::Debug("[TextureManager] Can't get texture %s, wasn't loaded...", fileName.c_str() );
        return NULL;
    }
}
Texture* TextureManager::GetTextureForID( const int serialID ) {
    if ( serialID > (int)textureMap.size() || serialID < 0 ) { return NULL; }
    std::map<std::string, Texture*>::iterator i = textureMap.begin();
    std::advance(i, serialID);
    return i->second;
}
std::string TextureManager::GetName(const Texture *tex) {
    // Check if this texture is loaded
    std::map<std::string, Texture*>::iterator i = textureMap.begin();
    while( i != textureMap.end() ) {
        if (i->second == tex) { return i->first; }
        i++;
    }
    Log::Debug("[TextureManager] Can't get name of texture %s, wasn't loaded...", tex->GetID() );
    return "";
}
std::string TextureManager::GetNameForID( const int serialID ) {
    if ( serialID > (int)textureMap.size() || serialID < 0 ) { return ""; }
    std::map<std::string, Texture*>::iterator i = textureMap.begin();
    std::advance(i, serialID);
    return i->first;
}
int TextureManager::GetSerialisedID ( const Texture* tex ) {
    int serialID = 0;
    std::map<std::string, Texture*>::iterator i = textureMap.begin();
    while( i != textureMap.end() ) {
        if ( i->second == tex ) { return serialID; }
        i++;
        serialID++;
    }
    return -1;
}
int TextureManager::GetSerialisedID ( const std::string fileName ) {
    int serialID = 0;
    std::map<std::string, Texture*>::iterator i = textureMap.begin();
    while( i != textureMap.end() ) {
        if (i->first == fileName) { return serialID; }
        i++;
        serialID++;
    }
    return -1;
}
bool TextureManager::UnloadTexture( const std::string fileName ) {
    bool result(true);
    if( textureMap.find(fileName) != textureMap.end() ) {
        // If this texture ID is already mapped
        if ( textureMap[fileName]->refCount == 0 ) {
            // unload it's texture, and remove it from the map
            textureMap[fileName]->Unload();
            Log::Debug("[TextureMan] Releasing texture %s\n", fileName.c_str());
            delete textureMap[fileName];
            textureMap.erase(fileName);
        } else {
            Log::Debug("[TextureMan] Not unloading texture %s yet, refcount: %i\n",
                       fileName.c_str(), textureMap[fileName]->refCount);
        }
    } else {
        // Otherwise unload failed
        result = false;
    }
    return result;
}

bool TextureManager::UnloadTexture(Texture* tex)
{
    std::map<std::string, Texture*>::iterator i = textureMap.begin();
    while( i != textureMap.end() ) {
        if ( i->second == tex ) {
            if ( textureMap[i->first]->refCount == 0 ) {
                textureMap[i->first]->Unload();
                Log::Debug("[TextureMan] Releasing texture %s\n",
                           i->first.c_str());
				                textureMap.erase(i->first);
                delete tex;
                return true;
            } else {
                Log::Debug("[TextureMan] Not releasing texture %s yet, refcount: %i\n",
                           i->first.c_str(), textureMap[i->first]->refCount);
            }
        }
        i++;
    }
    return false;
}

bool TextureManager::BindTexture(const std::string fileName)
{
    bool result(true);
    // If this texture ID is mapped, bind it's texture as current
    if( textureMap.find(fileName) != textureMap.end() )
    { textureMap[fileName]->Bind(); }
    // Otherwise binding failed
    else { result = false; }
    return result;
}

void TextureManager::UnloadAllTextures()
{
    // Start at the begginning of the texture map
    std::map<std::string, Texture*>::iterator i = textureMap.begin();
    
    // Unload the textures until the end of the texture map is found
    while(i != textureMap.end()) {
        if ( i->second->refCount == 0 ) {
            i->second->Unload();
            delete i->second;
        }
        Log::Debug("Not unloading texture %s due to refcount %i",
                   i->first.c_str(), textureMap[i->first]->refCount );
        i++;
    }
    // Clear the texture map
    textureMap.clear();
}

void TextureManager::UnloadTextures()
{
    Log::Debug("[TextureManager] Unloading %i textures", textureMap.size() );
    // Start at the begginning of the texture map
    std::map<std::string, Texture*>::iterator i = textureMap.begin();
    
    // First unload the textures
    while(i != textureMap.end()) {
        i->second->Unload();
        i++;
    }
}

void TextureManager::ReloadTextures()
{
    std::map<std::string, Texture*>::iterator i = textureMap.begin();

    // Then reload the textures
    while(i != textureMap.end()) {
        // Take the directory path for textures and append the filename
        std::string fullPath = i->second->GetPath();
        fullPath.append( i->first );
        i->second->LoadFromFile( fullPath );
        i++;
    }
    Log::Debug("[TextureManager] Reloaded %i textures", textureMap.size() );
}
