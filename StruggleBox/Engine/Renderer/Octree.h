#pragma once

#include "Allocator.h"
#include "GFXDefines.h"

class Octree
{
public:
	Octree(const uint8_t levels, Allocator& allocator);
	~Octree();

	void setLeafOccupied(const glm::ivec3& coord, const bool occupied);

	uint32_t getLevelDataSize(const uint8_t level) const;
	uint32_t getLevelDataOffset(const uint8_t level) const;
	uint32_t getLevelNumCellsPerSide(const uint8_t level) const;
	uint32_t getOctantOffset(const glm::ivec3& coord, const uint8_t level) const;

	const uint8_t getLevels() const { return m_levels; }
	const uint8_t* getOccupancyBitfield() const { return m_occupancyBitfield; }

private:
	uint8_t m_levels;
	uint8_t* m_occupancyBitfield;

	void setOctantOccupied(const glm::ivec3& coord, const bool occupied, uint8_t& bitfield);
};
