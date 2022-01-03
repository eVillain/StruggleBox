#pragma once

#include "Node.h"
#include "RendererDefines.h"
#include "Rect2D.h"
#include "Color.h"
#include <string>

class Renderer;
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

class LayoutNode : public Node
{
public:
    LayoutNode();
    //virtual ~LayoutNode();

    //void addChild(Node* child) override;
    //void removeChild(Node* child) override;

    void refresh();

    void setLayoutType(LayoutType type) { m_type = type; }
    void setPadding(float padding) { m_padding = padding; }
    void setAlignmentH(LayoutAlignmentH alignment) { m_alignmentH = alignment; }
    void setAlignmentV(LayoutAlignmentV alignment) { m_alignmentV = alignment; }

private:
    LayoutType m_type;
    float m_padding;
    LayoutAlignmentH m_alignmentH;
    LayoutAlignmentV m_alignmentV;

    float getStartPos() const;
};
