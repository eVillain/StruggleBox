#pragma once

#include "Node.h"
#include "RendererDefines.h"
#include "Rect2D.h"
#include "Color.h"
#include <string>

class SpriteBatch;
class Texture;

enum class LayoutType {
    Row,
    Column
};

enum class LayoutAlignmentH {
    Left,
    Middle,
    Right
};

enum class LayoutAlignmentV {
    Bottom,
    Middle,
    Top
};

enum class LayoutDirectionV {

};

class LayoutNode : public Node
{
public:
    LayoutNode();

    void refresh();

    void setLayoutType(LayoutType type) { m_type = type; }
    void setPadding(float padding) { m_padding = padding; }
    void setAlignmentH(LayoutAlignmentH alignment) { m_alignmentH = alignment; }
    void setAlignmentV(LayoutAlignmentV alignment) { m_alignmentV = alignment; }

    const glm::vec2 getUsedSize() const { return m_usedSize; }
private:
    LayoutType m_type;
    float m_padding;
    LayoutAlignmentH m_alignmentH;
    LayoutAlignmentV m_alignmentV;
    glm::vec2 m_usedSize;

    const float getRequiredWidth() const;
    const float getRequiredHeight() const;
    float getStartPos() const;
};
