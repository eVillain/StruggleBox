#pragma once

#include "Node.h"
#include "RendererDefines.h"
#include "Rect2D.h"
#include "Color.h"
#include <string>

class Renderer;
class SpriteBatch;
class Texture;

class SpriteNode : public Node
{
public:
    static Rect2D DEFAULT_9SLICE_INSETS;

    SpriteNode(const TextureID textureID, const Rect2D& texRect);
    SpriteNode(const TextureID textureID, const Rect2D& texRect, const Rect2D& insets, const glm::vec2& originalSize);

    ~SpriteNode();

    void draw(Renderer& renderer, const glm::vec3& parentPosition, const glm::vec2& parentScale) override;
    
    TextureID getTextureID() const { return m_textureID; }
    const Rect2D& getTextureRect() const { return m_texRect; }

    void setTextureID(const TextureID textureID) { m_textureID = textureID; }
    void setTextureRect(const Rect2D& texRect) { m_texRect = texRect; }
    void set9SliceInsets(const Rect2D& insets) { m_insets = insets; }
    void setEnable9Slice(const bool enable) { m_enable9Slice = enable; }
    void setOriginalSize(const glm::ivec2& originalSize) { m_originalSize = originalSize; }

private:
    TextureID m_textureID;
    Rect2D m_texRect;
    Rect2D m_insets;
    bool m_enable9Slice;
    glm::vec2 m_originalSize;

    void bufferQuad(
        const glm::vec2& bottomLeft,
        const glm::vec2& topRight,
        const glm::vec2& texBottomLeft,
        const glm::vec2& texTopRight,
        const float depth,
        Renderer& renderer) const;
};
