#ifndef FONT_ATLAS_POOL_H
#define FONT_ATLAS_POOL_H

#include "FontAtlas.h"
#include <map>
#include <string>
#include <memory>

// Map of FontAtlases with font filename as key and iterator
typedef std::map<std::string, std::shared_ptr<FontAtlas>> AtlasMap;
typedef AtlasMap::const_iterator AtlasIterator;

class FontAtlasPool
{
    friend class Text;
public:
    std::shared_ptr<FontAtlas> GetAtlas(const std::string filename,
                                        const int size);
    
protected:
    FontAtlasPool();
    bool Initialize();
    bool Terminate();
    
private:
    // FreeType library
    FT_Library _freeType;
    
    // Container for fonts
    AtlasMap _atlases;
};

#endif /* FONT_ATLAS_POOL_H */
