#include "SpriteNode.h"

#include "Renderer.h"

Rect2D SpriteNode::DEFAULT_9SLICE_INSETS = Rect2D(1.f / 3.f, 1.f / 3.f, 1.f / 3.f, 1.f / 3.f);

SpriteNode::SpriteNode(const TextureID textureID, const Rect2D& texRect)
	: m_textureID(textureID)
	, m_texRect(texRect)
	, m_insets(DEFAULT_9SLICE_INSETS)
	, m_enable9Slice(false)
	, m_originalSize(texRect.Size())
{
}
SpriteNode::SpriteNode(const TextureID textureID, const Rect2D& texRect, const Rect2D& insets, const glm::vec2& originalSize)
	: m_textureID(textureID)
	, m_texRect(texRect)
	, m_insets(insets)
	, m_enable9Slice(true)
	, m_originalSize(originalSize)
{
}

SpriteNode::~SpriteNode()
{
}

void SpriteNode::draw(Renderer& renderer, const glm::vec3& parentPosition, const glm::vec2& parentScale)
{
	const glm::vec2 scaledContentSize = m_contentSize * m_scale * parentScale;
	const glm::vec2 anchorOffset = scaledContentSize * m_anchorPoint;
	const glm::vec2 bottomLeft = glm::vec2(parentPosition.x, parentPosition.y) + glm::vec2(m_position.x, m_position.y) - anchorOffset;
	const glm::vec2 topRight = bottomLeft + scaledContentSize;
	const float positionZ = parentPosition.z + m_position.z;

	if (!m_enable9Slice)
	{
		bufferQuad(bottomLeft, topRight, glm::vec2(m_texRect.x, m_texRect.y), glm::vec2(m_texRect.Right(), m_texRect.Top()), positionZ, renderer);
	}
	else
	{
		const glm::vec2 scaledOriginalSize = m_originalSize * m_scale * parentScale; // This is the total size of the output
		const glm::vec2 bottomLeftSize = scaledOriginalSize * glm::vec2(m_insets.x, m_insets.y);
		const glm::vec2 topRightSize = scaledOriginalSize * glm::vec2(1.f - m_insets.Right(), 1.f - m_insets.Top());
		const glm::vec2 middleSize = scaledContentSize - (bottomLeftSize + topRightSize);
		const glm::vec2 slice1 = bottomLeft + bottomLeftSize;
		const glm::vec2 slice2 = slice1 + middleSize;
		const glm::vec2 slice1Tex = glm::vec2(m_texRect.x, m_texRect.y) + glm::vec2(m_texRect.w * m_insets.x, m_texRect.h * m_insets.y);
		const glm::vec2 slice2Tex = slice1Tex + glm::vec2(m_texRect.w * m_insets.w, m_texRect.h * m_insets.h);

		bufferQuad(bottomLeft, slice1, glm::vec2(m_texRect.x, m_texRect.y), slice1Tex, positionZ, renderer);
		bufferQuad(glm::vec2(slice1.x, bottomLeft.y), glm::vec2(slice2.x, slice1.y), glm::vec2(slice1Tex.x, m_texRect.y), glm::vec2(slice2Tex.x, slice1Tex.y), positionZ, renderer);
		bufferQuad(glm::vec2(slice2.x, bottomLeft.y), glm::vec2(topRight.x, slice1.y), glm::vec2(slice2Tex.x, m_texRect.y), glm::vec2(m_texRect.Right(), slice1Tex.y), positionZ, renderer);

		bufferQuad(glm::vec2(bottomLeft.x, slice1.y), glm::vec2(slice1.x, slice2.y), glm::vec2(m_texRect.x, slice1Tex.y), glm::vec2(slice1Tex.x, slice2Tex.y), positionZ, renderer);
		bufferQuad(glm::vec2(slice1.x, slice1.y), glm::vec2(slice2.x, slice2.y), glm::vec2(slice1Tex.x, slice1Tex.y), glm::vec2(slice2Tex.x, slice2Tex.y), positionZ, renderer);
		bufferQuad(glm::vec2(slice2.x, slice1.y), glm::vec2(topRight.x, slice2.y), glm::vec2(slice2Tex.x, slice1Tex.y), glm::vec2(m_texRect.Right(), slice2Tex.y), positionZ, renderer);

		bufferQuad(glm::vec2(bottomLeft.x, slice2.y), glm::vec2(slice1.x, topRight.y), glm::vec2(m_texRect.x, slice2Tex.y), glm::vec2(slice1Tex.x, m_texRect.Top()), positionZ, renderer);
		bufferQuad(glm::vec2(slice1.x, slice2.y), glm::vec2(slice2.x, topRight.y), glm::vec2(slice1Tex.x, slice2Tex.y), glm::vec2(slice2Tex.x, m_texRect.Top()), positionZ, renderer);
		bufferQuad(glm::vec2(slice2.x, slice2.y), glm::vec2(topRight.x, topRight.y), glm::vec2(slice2Tex.x, slice2Tex.y), glm::vec2(m_texRect.Right(), m_texRect.Top()), positionZ, renderer);
	}

	Node::draw(renderer, parentPosition, parentScale);
}

void SpriteNode::bufferQuad(
	const glm::vec2& bottomLeft,
	const glm::vec2& topRight,
	const glm::vec2& texBottomLeft,
	const glm::vec2& texTopRight,
	const float depth,
	Renderer& renderer) const
{
	TexturedVertexData* verts = renderer.queueTexturedVerts(6, m_textureID);

	verts[0].pos = glm::vec3(bottomLeft.x, bottomLeft.y, depth);
	verts[1].pos = glm::vec3(topRight.x, topRight.y, depth);
	verts[2].pos = glm::vec3(bottomLeft.x, topRight.y, depth);
	verts[3].pos = glm::vec3(bottomLeft.x, bottomLeft.y, depth);
	verts[4].pos = glm::vec3(topRight.x, bottomLeft.y, depth);
	verts[5].pos = glm::vec3(topRight.x, topRight.y, depth);

	verts[0].uv = glm::vec2(texBottomLeft.x, texBottomLeft.y);
	verts[1].uv = glm::vec2(texTopRight.x, texTopRight.y);
	verts[2].uv = glm::vec2(texBottomLeft.x, texTopRight.y);
	verts[3].uv = glm::vec2(texBottomLeft.x, texBottomLeft.y);
	verts[4].uv = glm::vec2(texTopRight.x, texBottomLeft.y);
	verts[5].uv = glm::vec2(texTopRight.x, texTopRight.y);
}
