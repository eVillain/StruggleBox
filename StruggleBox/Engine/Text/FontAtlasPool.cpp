#include "FontAtlasPool.h"
#include "PathUtil.h"
#include "Log.h"

FontAtlasPool::FontAtlasPool() :
_freeType()
{ }

bool FontAtlasPool::Initialize()
{
    // Initialize FreeType library
    if( FT_Init_FreeType(&_freeType) != 0 ) {
        Log::Error("[FontAtlasPool] Could not initialize FreeType library!");
        return false;
    }
    return true;
}

bool FontAtlasPool::Terminate()
{
    AtlasIterator atl;
    for ( atl = _atlases.begin(); atl != _atlases.end(); atl++ ) {
        Log::Debug("[FontAtlasPool] Atlas: %s", atl->first.c_str());
        Log::Debug("[FontAtlasPool] Use count: %l", atl->second.use_count());
    }
    _atlases.clear();
    FT_Done_FreeType(_freeType);
    return true;
}

/**
 Sees if the font atlas at that size was generated. If not, generates it.
 Should always return a valid font atlas if filename and size are legit
 */
std::shared_ptr<FontAtlas> FontAtlasPool::GetAtlas(const std::string filename,
                                                   const int size)
{
    char buf[256];
#ifdef _WIN32   /* Windows ಠ_ಠ I don't even */
    sprintf_s(buf, "%s%i", filename.c_str(), size);
#else
    sprintf(buf, "%s%i", filename.c_str(), size);
#endif
    // Check if we already have an atlas for that font at that size
    std::string fontKey = std::string(buf);
    AtlasIterator result = _atlases.find(fontKey);
    if( result != _atlases.end() ) {
        return result->second;
    }
    
    // We need to load the font and create a new atlas, let's do it
    std::string path = PathUtil::GetFontsPath() + filename;
    Log::Debug("[FontAtlasPool] Loading font from file: %s", path.c_str());
    
    FT_Face face = NULL;
    FT_Error err = FT_New_Face(_freeType, path.c_str(), 0, &face);
    if( err ) {
        Log::Error("[FontAtlasPool] Could not open font: %s - error: %i", path.c_str(), err);
        return NULL;
    }
    std::shared_ptr<FontAtlas> atlas = std::make_shared<FontAtlas>(face, size);
    
    // Store newly created atlas
    _atlases[fontKey] = atlas;
    
    // Clean up face
    FT_Done_Face(face);
    
    return atlas;
}
