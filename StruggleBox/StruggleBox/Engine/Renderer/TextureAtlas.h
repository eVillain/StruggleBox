#pragma once

#include "Texture2D.h"
#include "RendererDefines.h"
#include "Rect2D.h"
#include <map>
#include <string>
#include <stdint.h>

class TextureAtlas
{
public:
    TextureAtlas(
        const TextureID textureID,
        const uint32_t textureWidth,
        const uint32_t textureHeight,
        const std::map<std::string, Rect2D>& frames);
    ~TextureAtlas();

    const Rect2D& getRectForFrame(const std::string& frame) const;
    const Rect2D getTexRectForFrame(const std::string& frame) const;
    
    const TextureID getTextureID() const { return m_textureID; }
    const std::map<std::string, Rect2D>& getFrames() const { return m_frames; }

private:
    static Rect2D UNIT_RECT;

    const TextureID m_textureID;
    const uint32_t m_textureWidth;
    const uint32_t m_textureHeight;
    std::map<std::string, Rect2D> m_frames;
};
