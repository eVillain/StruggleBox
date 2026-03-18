#include "Octree.h"


Octree::Octree(const uint8_t levels, Allocator& allocator)
: m_levels(levels)
, m_occupancyBitfield(nullptr)
{
	const uint32_t octreeBitfieldSize = getLevelDataOffset(m_levels);
	m_occupancyBitfield = (uint8_t*)allocator.allocate(octreeBitfieldSize);
	memset(m_occupancyBitfield, 0, octreeBitfieldSize);
}

Octree::~Octree()
{
}

void Octree::setLeafOccupied(const glm::ivec3& coord, const bool occupied)
{
	Log::Debug("[Octree::setLeafOccupied] coord: %i, %i, %i", coord.x, coord.y, coord.z);
	// Propagate changes up the tree
	glm::ivec3 lastCoord = coord;
	for (int8_t level = m_levels - 1; level >= 0; level--)
	{
		const glm::ivec3 localCoord = lastCoord / 2;
		const uint32_t octantOffset = getOctantOffset(localCoord, level);
		const glm::ivec3 octantCoord = glm::ivec3(lastCoord.x % 2, lastCoord.y % 2, lastCoord.z % 2);
		setOctantOccupied(octantCoord, occupied, m_occupancyBitfield[octantOffset]);
		Log::Debug("level %i, octant offset: %i, octant coord: %i, %i, %i", level, octantOffset, octantCoord.x, octantCoord.y, octantCoord.z);
		Log::Debug("local coord %i, %i, %i, bits: %i", localCoord.x, localCoord.y, localCoord.z, m_occupancyBitfield[octantOffset]);
		lastCoord = localCoord;
	}
}

uint32_t Octree::getLevelDataSize(const uint8_t level) const
{
	if (level == 0) return 1;
	return std::pow(8, level);
}

uint32_t Octree::getLevelDataOffset(const uint8_t level) const
{
	if (level == 0) return 0;
	uint32_t octreeBitfieldSize = 0;
	for (int l = 0; l < level; l++)
	{
		octreeBitfieldSize += getLevelDataSize(l);
	}
	return octreeBitfieldSize;
}

uint32_t Octree::getLevelNumCellsPerSide(const uint8_t level) const
{
	return pow(2, level);
}

uint32_t Octree::getOctantOffset(const glm::ivec3& coord, const uint8_t level) const
{
	const uint32_t levelOffset = getLevelDataOffset(level);
	const uint32_t levelWidth = getLevelNumCellsPerSide(level);
	return levelOffset + coord.x + (coord.y * levelWidth) + (coord.z * levelWidth * levelWidth);
}

void Octree::setOctantOccupied(const glm::ivec3& coord, const bool occupied, uint8_t& bitfield)
{
	const uint8_t bit = coord.x + (coord.y * 2) + (coord.z * 4);
	if (occupied) bitfield |= (1 << bit);
	else bitfield & ~((uint8_t)1 << bit);
}
