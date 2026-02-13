#pragma once

#define VOXELDATA_HEADER "VxlDt"
#define VOXELDATA_VERSION "0.1"

#include "GFXDefines.h"
#include "VoxelRenderer.h"
//#include <btBulletDynamicsCommon.h>
#include <cstdint>
#include <vector>

class Allocator;
class Physics;
//class VoxelRenderer;
struct Coord3D;

const uint8_t EMPTY_VOXEL = 0;

class VoxelData
{
public:
	VoxelData(uint16_t sizeX, uint16_t sizeY, uint16_t sizeZ, Allocator& allocator);
	~VoxelData();

	void setData(const uint8_t* data, const size_t size);
	uint8_t* getData() { return _data; }
	const uint8_t* getData() const { return _data; }
	
	uint8_t& operator[] (int index) { return _data[index]; }
	uint8_t& operator() (int x, int y, int z) { return _data[getIndex(x,y,z)]; }

	void setScale(float scale) { _scale = scale; }
	float getScale() const { return _scale; }

	void clear();
	void fill(const uint8_t type);
	void replaceType(const uint8_t oldType, const uint8_t newType);
	void rotateY(const bool ccw);

	bool isEmpty() const;

	void generateGrass(const int seed);
	void generateTree(const glm::vec3 treePos, const int seed);
	void generateTowerChunk(const Coord3D& coord);


	void createTriangleMesh(VoxelRenderer& renderer, const DrawDataID drawDataID, const float radius) const;
	void createTriangleMeshReduced(VoxelMeshPBRVertexData* verts, size_t& vertexCount, const float radius) const;
	//TexturedPBRVertexData* createTriangleMeshReduced(Renderer3DDeferred& renderer, uint32_t& vertexCount, const float radius) const;
	//void getMeshLinear(Mesh& mesh, float radius);
	//void getMeshLinearWithVertexAO(Mesh& mesh, float radius);
	//void getMeshReduced(Mesh& mesh, float radius);

	const uint32_t getPhysicsReduced(Physics& physics, float radius) const;
	const uint32_t getPhysicsCubes(Physics& physics, float radius) const;
	const uint32_t getPhysicsHull(Physics& physics, const float radius) const;
	const uint32_t getPhysicsAABBs(Physics& physics, const glm::vec3& radius) const;

	bool contains(const int x, const int y, const int z) const
	{
		return (
			x >= 0 && y >= 0 && z >= 0 &&
			x < _sizeX && y < _sizeY && z < _sizeZ);
	}

	const uint16_t getSizeX() const { return _sizeX; }
	const uint16_t getSizeY() const { return _sizeY; }
	const uint16_t getSizeZ() const { return _sizeZ; }

	const int getIndex(const glm::ivec3 coord) const { return linearIndexFromCoordinate(coord.x, coord.y, coord.z, _sizeX, _sizeY); }
	const int getIndex(const int x, const int y, const int z) const { return linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY); }

	const glm::vec3 getVolume(const float radius) const { return glm::vec3(_sizeX, _sizeY, _sizeZ)*radius*2.0f; }

private:

	struct SurfaceRect {
		uint8_t voxel;
		glm::ivec2 origin;
		glm::ivec2 size;
	};
	void extractFaceRect(
		const glm::ivec3 coord,
		const glm::ivec3 blockerCoord,
		uint8_t& previousVoxel,
		bool& previousFaceVisible,
		std::vector<SurfaceRect>& rects) const;
	void mergeRects(
		const std::vector<SurfaceRect>& rectsForRow,
		std::vector<SurfaceRect>& rectsForSlice) const;
	glm::vec3 voxelCoordForAxis(const SurfaceRect& rect, const int axis, const int slice) const;
	glm::vec3 voxelCornerForAxis(const SurfaceRect& rect, const int axis) const;
	void createFaceVerts(
		const SurfaceRect& rect,
		const int slice,
		const int axis,
		const float radius,
		VoxelMeshPBRVertexData* verts,
		size_t& vertCount) const;


	Allocator& m_allocator;
	uint16_t _sizeX, _sizeY, _sizeZ;

	uint8_t* _data; // Raw voxel data, each byte is one voxel type ID
	float _scale; // Object scale - 1.0 = one world-sized block (256px per side)

	static int linearIndexFromCoordinate(
		const int x,
		const int y,
		const int z,
		const int max_x,
		const int max_y)
	{
		int a = 1;
		int	b = max_x;
		int	c = max_x * max_y;
		int	d = 0;
		return a*x + b*y + c*z + d;
	}

	static glm::ivec3 coordinateFromLinearIndex(
		const int idx,
		const int max_x,
		const int max_y)
	{
		int tmpIdx = idx;
		int x = tmpIdx % max_x;
		tmpIdx /= max_x;
		int	y = tmpIdx % max_y;
		tmpIdx /= max_y;
		int	z = tmpIdx;
		return glm::ivec3(x, y, z);
	}

	bool blocksVisibility(int x, int y, int z) const
	{
		if (!contains(x, y, z))
			return false;

		int idx = linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY);
		uint8_t& b = _data[idx];

		// Non-transparent blocks always block line of sight
		if (b > EMPTY_VOXEL) { return true; }
		return false;
	}

	bool blocksPhysics(int x, int y, int z) const
	{
		if (!contains(x, y, z))
			return false;

		int idx = linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY);
		uint8_t& b = _data[idx];

		// Non-transparent blocks always block line of sight
		if (b > EMPTY_VOXEL) { return true; }
		return false;
	}
};
