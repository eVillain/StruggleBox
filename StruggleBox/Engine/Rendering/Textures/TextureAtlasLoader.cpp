#include "TextureAtlasLoader.h"

#include "Allocator.h"
#include "ArenaOperators.h"
#include "Log.h"
#include "Texture2D.h"
#include "TextureAtlas.h"
#include "Dictionary.h"
#include "Renderer.h"
#include "FileUtil.h"
#include "Rect2D.h"
#include <map>

TextureAtlas* AtlasLoader::load(const std::string& atlasFile, Allocator& allocator, Renderer& renderer)
{
    Dictionary rootDict;
    if (!rootDict.loadRootSubDictFromFile(atlasFile.c_str()))
    {
        Log::Error("[AtlasLoader] Failed to load atlas file %s", atlasFile.c_str());
        return nullptr;
    }
    if (!rootDict.stepIntoSubDictWithKey("metadata"))
    {
        Log::Error("[AtlasLoader] Failed to load metadata for file %s", atlasFile.c_str());
        return nullptr;
    }

    const std::string atlasFolder = FileUtil::GetContainingFolder(atlasFile);
    const std::string textureFileName = atlasFolder + rootDict.getStringForKey("textureFileName");

    const TextureID textureID = renderer.getTextureID(textureFileName, true);
    const Texture2D* texture = renderer.getTextureByID(textureID);

    if (texture == nullptr)
    {
        Log::Error("[AtlasLoader] Failed to load texture file %s", textureFileName.c_str());
        return nullptr;
    }

    rootDict.stepOutOfSubDict();
    if (!rootDict.stepIntoSubDictWithKey("frames"))
    {
        Log::Error("[AtlasLoader] Failed to load frames for file %s", atlasFile.c_str());
        return nullptr;
    }

    const std::vector<std::string> frameNames = rootDict.getAllKeys();
    std::map<std::string, Rect2D> frames;
    for (size_t i = 0; i < frameNames.size(); i++)
    {
        if (!rootDict.stepIntoSubDictWithKey(frameNames[i].c_str()))
        {
            Log::Error("[AtlasLoader] Error parsing frame %s in file %s", frameNames.at(i).c_str(), atlasFile.c_str());
            return nullptr;
        }
        Rect2D r = rootDict.getRectForKey("frame");
        if (r.x == 0 && r.y == 0 && r.w == 0 && r.h == 0)
        {
            r = rootDict.getRectForKey("textureRect");
        }
        rootDict.stepOutOfSubDict();

        frames[frameNames[i]] = r;
        Log::Info("[AtlasLoader] Read frame %s - rect %f, %f, %f, %f", frameNames.at(i).c_str(), r.x, r.y, r.w, r.h);
    }

    bool debugInfo = true;
    if (debugInfo)
    {
        int format = rootDict.getIntegerForKey("format");
        int numFrames = rootDict.getNumKeys();
        Log::Info("[AtlasLoader] Loaded .plist - format %i, frame count %i", format, numFrames);
    }

    TextureAtlas* atlas = CUSTOM_NEW(TextureAtlas, allocator)(textureID, texture->getWidth(), texture->getHeight(), frames);
    return atlas;
}
