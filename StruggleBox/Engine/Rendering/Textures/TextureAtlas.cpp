#include "TextureAtlas.h"

#include "Log.h"

Rect2D TextureAtlas::UNIT_RECT = Rect2D(0.f, 0.f, 1.f, 1.f);

TextureAtlas::TextureAtlas(
    const TextureID textureID,
    const uint32_t textureWidth,
    const uint32_t textureHeight,
    const std::map<std::string, Rect2D>& frames)
    : m_textureID(textureID)
    , m_textureWidth(textureWidth)
    , m_textureHeight(textureHeight)
    , m_frames(frames)
{
}

TextureAtlas::~TextureAtlas()
{
}

const Rect2D& TextureAtlas::getRectForFrame(const std::string& frame) const
{
    auto it = m_frames.find(frame);
    if (it == m_frames.end())
    {
        return UNIT_RECT;
    }
    
    return it->second;
}

const Rect2D TextureAtlas::getTexRectForFrame(const std::string& frame) const
{
    auto it = m_frames.find(frame);
    if (it == m_frames.end())
    {
        return UNIT_RECT;
    }

    const Rect2D& r = it->second;

    const float tw = r.w / (m_textureWidth);
    const float th = r.h / (m_textureHeight);
    const float tx = r.x / (m_textureWidth);
    const float ty = 1.0f - (r.y / (m_textureHeight)) - th;
    return Rect2D(tx, ty, tw, th);
}
