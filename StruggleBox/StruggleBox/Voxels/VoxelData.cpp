#include "VoxelData.h"

#include "Allocator.h"
#include "ArenaOperators.h"
#include "Coord.h"
#include "GFXDefines.h"
//#include "MaterialData.h"
#include "Serialise.h"
#include "CubeConstants.h"
#include "Log.h"
#include "Physics.h"
#include "VoxelAABB.h"
#include "VoxelRenderer.h"
#include <cstdlib>
#include <iterator>

VoxelData::VoxelData(uint16_t sizeX, uint16_t sizeY, uint16_t sizeZ, Allocator& allocator)
	: m_allocator(allocator)
	, _sizeX(sizeX)
	, _sizeY(sizeY)
	, _sizeZ(sizeZ)
	, _scale(1.f)
	, _data(nullptr)
{
	const size_t dataSize = (size_t)_sizeX * (size_t)_sizeY * (size_t)_sizeZ;
	_data = CUSTOM_NEW_ARRAY(uint8_t, dataSize, allocator);
	clear();
}

VoxelData::~VoxelData()
{
	CUSTOM_DELETE_ARRAY(_data, m_allocator);
}

void VoxelData::setData(const uint8_t * data, const size_t size)
{
	const size_t dataSize = (size_t)_sizeX * (size_t)_sizeY * (size_t)_sizeZ;
	if (size > dataSize)
	{
		Log::Error("[VoxelData] Can't set data, too large! (size: %i, data: %i)",
			size, dataSize);
		return;
	}
	memcpy(_data, data, size);
}

void VoxelData::clear()
{
	const size_t dataSize = (size_t)_sizeX * (size_t)_sizeY * (size_t)_sizeZ;
	memset(_data, 0, dataSize);
}

void VoxelData::fill(const uint8_t type)
{
	const size_t dataSize = (size_t)_sizeX * (size_t)_sizeY * (size_t)_sizeZ;
	memset(_data, type, dataSize);
}

void VoxelData::replaceType(const uint8_t oldType, const uint8_t newType)
{
	const int numVoxels = _sizeX*_sizeY*_sizeZ;
	int numVerts = 0;
	for (int i = 0; i < numVoxels; i++)
	{
		uint8_t& b = _data[i];
		if (b == oldType)
			b = newType;
	}
}

void VoxelData::rotateY(const bool ccw)
{
	const int numVoxels = _sizeX*_sizeY*_sizeZ;
	uint8_t* tempBlocks = CUSTOM_NEW_ARRAY(uint8_t, numVoxels, m_allocator);

	for (int x = 0; x < _sizeX; x++)
	{
		for (int z = 0; z < _sizeZ; z++)
		{
			for (int y = 0; y < _sizeY; y++)
			{
				const int index = linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY);
				int tempX = ccw ? z : (_sizeX - 1) - z;
				int tempZ = ccw ? (_sizeZ - 1) - x : x;
				int tempPos = linearIndexFromCoordinate(tempX, y, tempZ, _sizeX, _sizeY);
				tempBlocks[tempPos] = _data[index];
			}
		}
	}

	memcpy(_data, tempBlocks, numVoxels);
	CUSTOM_DELETE_ARRAY(tempBlocks, m_allocator);
}

bool VoxelData::isEmpty() const
{
	const int numVoxels = _sizeX * _sizeY * _sizeZ;
	for (int index = 0; index < numVoxels; index++)
	{
		if (_data[index] != 0)
		{
			return false;
		}
	}
	return true;
}


#include "Random.h"
#include "Timer.h"
void VoxelData::generateTree(
	const glm::vec3 treePos,
	const int seed)
{
	Random::RandomSeed(seed);

	const bool wobblyTrunk = false;

	const uint8_t TRUNK_MATERIAL = 85;
	const uint8_t FOLIAGE_MATERIAL = 139;
	// TODO:: Make tree variables editable
	const float trunkHeight = 2.0f;
	const float trunkRadius = 2.0f;
	const float leafRadius = 8.0f;
	const float blockWidth = 0.5f;
	const float blockRadius = blockWidth * 0.5f;
	glm::vec2 trunkCenterXZ = glm::vec2(_sizeX * blockRadius, _sizeZ * blockRadius);

	for (int y = 0; y < _sizeY; y++)
	{
		const float heightRatio = (float)y / _sizeY;
		const float trunkShrink = heightRatio * trunkRadius;
		const float leafShrinkTree = (heightRatio * leafRadius);
		const float wut = (8.f * fmod(sinf((1.f-heightRatio) * M_PI_2), 0.125f));
		const float leafShrinkBranch = (wut * leafRadius) * (1.f-heightRatio);
		const float leafShrink = leafShrinkBranch;// std::max(leafShrinkTree, leafShrinkBranch);

		const float ringLeafRadius = leafShrinkBranch;
		if (wobblyTrunk)
		{
			const float trunkShiftAngle = Random::RandomDouble() * M_PI;
			const float trunkShiftAmount = (Random::RandomDouble() * 2.f) - 1.f;
			trunkCenterXZ += glm::vec2(sin(trunkShiftAngle), cos(trunkShiftAngle)) * trunkShiftAmount;
		}

		for (int x = 0; x < _sizeX; x++)
		{
			for (int z = 0; z < _sizeZ; z++)
			{
				const float posX = (x * blockWidth) + blockRadius;
				const float posZ = (z * blockWidth) + blockRadius;
				const float posY = (y * blockWidth) + blockRadius;
				const glm::vec3 cubePos = glm::vec3(posX, posY, posZ);
				const glm::vec2 cubeXZ = glm::vec2(posX, posZ);
				const float dist = glm::distance(cubeXZ, trunkCenterXZ);
				if (dist <= (trunkRadius - trunkShrink))
				{
					const int index = linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY);
					_data[index] = TRUNK_MATERIAL;
				}
				else
					if (posY >= trunkHeight &&
						dist <= (ringLeafRadius))
				{
					const int index = linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY);
					_data[index] = FOLIAGE_MATERIAL;
				}
			}
		}
	}




	//struct Branch {
	//	glm::vec3 pos;
	//	glm::vec3 end;
	//	float radius;
	//};
	//std::vector<Branch> branches;
	//int maxBranches = 4;
	//Branch trunk = { treePos, treePos + glm::vec3(0.0f, trunkHeight, 0.0f), trunkRadius };



	//branches.push_back(trunk);
	//// Generate trunk
	//for (int i = 0; i<maxBranches; i++) {
	//	if (i > branches.size() - 1) break;
	//	// Process branch
	//	Branch b = branches[i];
	//	glm::vec3 newPos = b.pos;
	//	float finishDist = glm::distance(b.pos, b.end);
	//	bool done = false;
	//	while (!done) {
	//		bool branched = false;
	//		int sides_b = b.radius / BLOCK_RADIUS;
	//		if (sides_b < 1) {
	//			int arrPos = PosToArray(newPos);
	//			if (blocks[arrPos].blockType == Type_Empty) {
	//				blocks[arrPos].blockType = Type_Wood_1;
	//				blocks[arrPos].blockColor = ColorForType(Type_Wood_1);
	//				if (i == 0) {
	//					float trunkCol = 0.8f;
	//					blocks[arrPos].blockColor.r *= trunkCol;
	//					blocks[arrPos].blockColor.g *= trunkCol;
	//					blocks[arrPos].blockColor.b *= trunkCol;
	//				}
	//			}
	//		}
	//		else {
	//			// Add blocks for current branch position
	//			for (int x = -sides_b; x<sides_b; x++) {
	//				for (int z = -sides_b; z<sides_b; z++) {
	//					for (int y = -sides_b; y<sides_b; y++) {
	//						float posX = newPos.x + (x*blockWidth) + BLOCK_RADIUS;
	//						float posZ = newPos.z + (z*blockWidth) + BLOCK_RADIUS;
	//						float posY = newPos.y + (y*blockWidth) + BLOCK_RADIUS;
	//						glm::vec3 cubePos = glm::vec3(posX, posY, posZ);
	//						if (glm::distance(cubePos, newPos) < b.radius) {
	//							int arrPos = PosToArray(cubePos);
	//							blocks[arrPos].blockType = Type_Wood_1;
	//							blocks[arrPos].blockColor = ColorForType(Type_Wood_1);
	//						}
	//					}   // for y
	//				}   // for z
	//			}   // for x
	//		}

	//		//        }   // while !done
	//		//    }   // for i < maxBranches
	//		//            if ( glm::distance(glm::vec2(treePos.x,treePos.z),glm::vec2(newPos.x,newPos.z)) > GetWidth()*BLOCK_RADIUS*0.5f ) done = true;
	//		// Update branch position
	//		glm::vec3 move = glm::normalize(b.end - newPos)*float(BLOCK_RADIUS);
	//		if (newPos.y + move.y < b.end.y) {
	//			float finishRatio = glm::distance(newPos, b.end) / finishDist;
	//			// Shrink radius
	//			double shrinkSize = Random::RandomDouble()*(1.0 - finishRatio)*0.025*b.radius;
	//			b.radius -= shrinkSize;
	//			if (b.radius < BLOCK_RADIUS*0.5f) { done = true; }
	//			// Check if we want to finish the branch
	//			double randomFinish = Random::RandomDouble()*0.1f;
	//			if (finishRatio - randomFinish < 0.0f) {
	//				// Split at random finish
	//				if (branches.size() < maxBranches) {
	//					float newBranchSize = b.radius*0.5f;
	//					double newBranchX = (Random::RandomDouble()*2.0 - 1.0);
	//					double newBranchY = (Random::RandomDouble()*2.0 - 1.0);
	//					double newBranchZ = (Random::RandomDouble()*2.0 - 1.0);
	//					glm::vec3 newEnd = glm::normalize(glm::vec3(newBranchX, newBranchY, newBranchZ))*finishDist*0.6f;
	//					Branch newBranch = { newPos, newEnd, newBranchSize };
	//					branches.push_back(newBranch);
	//					glm::vec3 newEnd2 = glm::normalize(glm::vec3(-newBranchX, newBranchY, -newBranchZ))*finishDist*0.6f;
	//					Branch newBranch2 = { newPos, newEnd2, newBranchSize };
	//					branches.push_back(newBranch2);
	//					branched = true;
	//				}
	//				done = true;
	//			}
	//			// Check if we want to deviate course
	//			double randomX = (Random::RandomInt(-1, 1))*BLOCK_RADIUS*0.5f;
	//			double randomZ = randomX == 0 ? (Random::RandomInt(-1, 1))*BLOCK_RADIUS*0.5f : 0;
	//			move.x += randomX;
	//			move.z += randomZ;
	//			// Check if we want to branch off
	//			if (branches.size() < maxBranches) {
	//				double newBranchRand = Random::RandomDouble()*(1.0 - finishRatio);
	//				if (newBranchRand > 0.5f || done) {
	//					float newBranchSize = b.radius*0.5f;
	//					double newBranchX = (Random::RandomDouble()*2.0 - 1.0);
	//					double newBranchY = (Random::RandomDouble()*2.0 - 1.0);
	//					double newBranchZ = (Random::RandomDouble()*2.0 - 1.0);
	//					glm::vec3 newEnd = glm::normalize(glm::vec3(newBranchX, newBranchY, newBranchZ))*finishDist*0.3f;
	//					Branch newBranch = { newPos, newEnd, newBranchSize };
	//					branches.push_back(newBranch);
	//					branched = true;
	//				}
	//			}
	//		}
	//		else {
	//			done = true;
	//		}
	//		// Generate leaves
	//		if (done && !branched) {
	//			double foliageX = leafRadius + (Random::RandomInt(1, 2))*blockWidth;
	//			double foliageY = leafRadius + (Random::RandomInt(1, 2))*blockWidth;
	//			// Create foliage at end of branch
	//			SetSphere(newPos, glm::vec3(foliageX, foliageY, foliageX), Type_Grass, false);
	//		}
	//		newPos += move;
	//	}   // while !done
	//}   // for i < maxBranches
}

#include <glm/gtc/noise.hpp> // glm::simplex
void VoxelData::generateGrass(const int seed)
{
	for (int x = 0; x < _sizeX; x++) {
		for (int z = 0; z < _sizeZ; z++) {
			float threshold = glm::simplex(glm::vec3(x*0.5f, seed, z*0.5f));
			if (threshold > 0.3f) {
				int height = threshold*_sizeY;
				for (int y = 0; y < height; y++) {
					int arrPos = linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY);
					if (arrPos != -1) {
						_data[arrPos] = 17;// GRASS VOXEL
					}
				}
			}
		}
	}
}

void VoxelData::generateTowerChunk(const Coord3D& coord)
{
	bool isEmpty = true;
	const glm::vec3 chunkPos = glm::vec3(coord.x * 16, coord.y * 16, coord.z * 16);
	for (int x = 0; x < _sizeX; x++)
	{
		for (int z = 0; z < _sizeZ; z++)
		{
			for (int y = 0; y < _sizeY; y++)
			{
				const glm::vec3 voxelPos = chunkPos + glm::vec3(x, y, z);
				const int index = linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY);
				if (voxelPos.y < -8.f)
				{
					_data[index] = 253; // Floor
				}
				else if ((int)voxelPos.y % 16 == 0)
				{
					_data[index] = 193; // Floor
				}
				else
				{
					const float distToCenter = glm::length(glm::vec2(voxelPos.x, voxelPos.z));
					if (distToCenter > 16.f)
					{
						_data[index] = 149; // Circular tower wall
					}
					//Log::Debug("Voxel at %f, %f, %f dist %f", voxelPos.x, voxelPos.y, voxelPos.z, distToCenter);
				}
			}
		}
	}
}

void VoxelData::generateFlatLand(const Coord3D& coord)
{
	const size_t dataSize = (size_t)_sizeX * (size_t)_sizeY * (size_t)_sizeZ;
	const uint8_t value = coord.y > 0 ? 0 : 1;
	memset(_data, value, dataSize);
}

void VoxelData::createTriangleMesh(VoxelMeshPBRVertexData* verts, size_t& vertexCount, const float radius, const glm::vec3& offset) const
{
	const float voxelWidth = radius * 2.0f;
	const glm::vec3 size = glm::vec3(_sizeX, _sizeY, _sizeZ);
	const glm::vec3 voxelOffset = glm::vec3(radius) - (size * radius);
	const size_t numVoxels = (size_t)_sizeX * (size_t)_sizeY * (size_t)_sizeZ;

	for (size_t i = 0; i < numVoxels; i++)
	{
		const uint8_t& b = _data[i];
		if (b == EMPTY_VOXEL)
		{
			continue;
		}
		const glm::ivec3 coord = coordinateFromLinearIndex(i, _sizeX, _sizeY);
		const glm::vec3 pos = (glm::vec3(coord.x, coord.y, coord.z) * voxelWidth) + voxelOffset;

		// Check visibility of each side
		const bool visibility_x_neg = !blocksVisibility(coord.x - 1, coord.y, coord.z);
		const bool visibility_x_pos = !blocksVisibility(coord.x + 1, coord.y, coord.z);
		const bool visibility_y_neg = !blocksVisibility(coord.x, coord.y - 1, coord.z);
		const bool visibility_y_pos = !blocksVisibility(coord.x, coord.y + 1, coord.z);
		const bool visibility_z_neg = !blocksVisibility(coord.x, coord.y, coord.z - 1);
		const bool visibility_z_pos = !blocksVisibility(coord.x, coord.y, coord.z + 1);
		const bool visibility[6] = { visibility_x_neg, visibility_x_pos, visibility_y_neg, visibility_y_pos, visibility_z_neg, visibility_z_pos };
		if (!visibility_x_neg && !visibility_x_pos &&
			!visibility_y_neg && !visibility_y_pos &&
			!visibility_z_neg && !visibility_z_pos)
		{
			continue;
		}

		const glm::vec3 positions[8] = {
			glm::vec3(pos.x - radius, pos.y - radius, pos.z - radius), // 0 left-bottom-rear
			glm::vec3(pos.x + radius, pos.y - radius, pos.z - radius), // 1 right-bottom-rear
			glm::vec3(pos.x - radius, pos.y + radius, pos.z - radius), // 2 left-top-rear
			glm::vec3(pos.x + radius, pos.y + radius, pos.z - radius), // 3 right-top-rear
			glm::vec3(pos.x - radius, pos.y - radius, pos.z + radius), // 4 left-bottom-front
			glm::vec3(pos.x + radius, pos.y - radius, pos.z + radius), // 5 right-bottom-front
			glm::vec3(pos.x - radius, pos.y + radius, pos.z + radius), // 6 left-top-front
			glm::vec3(pos.x + radius, pos.y + radius, pos.z + radius), // 7 right-top-front
		};

		for (size_t index = 0; index < 36; index++)
		{
			const int index_side = index / 6;
			if (!visibility[index_side])
			{
				continue;
			}

			const size_t index_vertex = CubeConstants::cube_indices[index];

			verts[vertexCount++] = {
				positions[index_vertex],
				glm::vec3(CubeConstants::cube_normals[index_side].x, CubeConstants::cube_normals[index_side].y, CubeConstants::cube_normals[index_side].z),
				HSVColor(index * 10.f, 1.f, (float)index / 35.f),
				//RGBAColor(1.f, 1.f, 1.f, 0.25f),
				glm::vec3(0, 0, 0)
			};
		}
	}
}

void VoxelData::extractFaceRect(
	const glm::ivec3 coord,
	const glm::ivec3 blockerCoord,
	uint8_t& previousVoxel,
	bool& previousFaceVisible,
	std::vector<SurfaceRect>& rects,
	const Axis axis) const
{
	const int voxelIndex = linearIndexFromCoordinate(coord.x, coord.y, coord.z, _sizeX, _sizeY);
	const uint8_t voxel = _data[voxelIndex];
	if (voxel == EMPTY_VOXEL)
	{
		previousVoxel = EMPTY_VOXEL;
		previousFaceVisible = false;
		return; // Voxel face not visible
	}

	if (!blocksPhysics(blockerCoord.x, blockerCoord.y, blockerCoord.z)) // face is visible
	{
		if (previousVoxel == voxel) // extend previous rect
		{
			SurfaceRect& previousRect = rects.back();
			previousRect.size.x += 1;
			//Log::Debug("Extract: %i (%i,%i,%i) extended (%i, %i) to (%i, %i)", voxelIndex, coord.x, coord.y, coord.z, previousRect.origin.x, previousRect.origin.y, previousRect.size.x, previousRect.size.y);
		}
		else
		{
			glm::ivec2 rectCoord;
			if (axis == Axis::Z)
			{
				rectCoord = glm::ivec2(coord.z, coord.y);
			}
			else if (axis == Axis::Y)
			{
				rectCoord = glm::ivec2(coord.x, coord.z);
			}
			else if (axis == Axis::X)
			{
				rectCoord = glm::ivec2(coord.x, coord.y);
			}
			SurfaceRect voxelRect = {
				voxel,
				rectCoord,
				glm::ivec2(1, 1)
			};
			//Log::Debug("Extract: %i (%i,%i,%i) created %i, %i to %i, %i", voxelIndex, coord.x, coord.y, coord.z, voxelRect.origin.x, voxelRect.origin.y, voxelRect.size.x, voxelRect.size.y);
			rects.push_back(voxelRect);
			previousVoxel = voxel;
		}
	}
	else
	{
		previousVoxel = EMPTY_VOXEL;
	}
}

void VoxelData::mergeRects(
	const std::vector<SurfaceRect>& rectsForRow,
	std::vector<SurfaceRect>& rectsForSlice) const
{
	for (const SurfaceRect& rowRect : rectsForRow)
	{
		auto it = std::find_if(rectsForSlice.begin(), rectsForSlice.end(), [&rowRect](const SurfaceRect& rect) {
			return rowRect.voxel == rect.voxel && // same type
				rowRect.origin.x == rect.origin.x && // same horizontal position
				rowRect.size.x == rect.size.x && // same size
				rowRect.origin.y == rect.origin.y + rect.size.y; // vertically stacked
			});
		if (it != rectsForSlice.end()) // Merge with previous row from current slice
		{
			(*it).size.y += 1;
			const SurfaceRect& previousRect = *it;
			//Log::Debug("Merge: merged %i, %i to %i, %i", previousRect.origin.x, previousRect.origin.y, previousRect.size.x, previousRect.size.y);
		}
		else
		{
			rectsForSlice.push_back(rowRect); // No merge possible, add new rect to slice
			//Log::Debug("Merge: added %i, %i to %i, %i", rowRect.origin.x, rowRect.origin.y, rowRect.size.x, rowRect.size.y);
		}
	}
}

glm::vec3 VoxelData::voxelCoordForAxis(const SurfaceRect& rect, const int axis, const int slice) const
{
	if (axis < 2)
	{
		return glm::vec3(slice, rect.origin.y, rect.origin.x);
	}
	if (axis < 4)
	{
		return glm::vec3(rect.origin.x, slice, rect.origin.y);
	}
	return glm::vec3(rect.origin.x, rect.origin.y, slice);
}

glm::vec3 VoxelData::voxelCornerForAxis(const SurfaceRect& rect, const int axis) const
{
	if (axis < 2)
	{
		return glm::vec3(1.f, rect.size.y, rect.size.x);
	}
	if (axis < 4)
	{
		return glm::vec3(rect.size.x, 1.f, rect.size.y);
	}
	return glm::vec3(rect.size.x, rect.size.y, 1.f);
}

void VoxelData::createFaceVerts(
	const SurfaceRect& rect,
	const int slice,
	const int axis,
	const float radius,
	const glm::vec3& offset,
	VoxelMeshPBRVertexData* verts,
	size_t& vertCount) const
{
	const glm::vec3 voxelCoord = voxelCoordForAxis(rect, axis, slice);
	const float voxelWidth = radius * 2.0f;
	const glm::vec3 size = glm::vec3(_sizeX, _sizeY, _sizeZ);
	//const glm::vec3 voxelUV3D = voxelCoord / size;
	const glm::vec3 voxelOffset = glm::vec3(radius) - (size * radius);
	const glm::vec3 voxelPos = (voxelCoord * voxelWidth) + voxelOffset + offset;
	const glm::vec3 voxelLBR = voxelPos - radius;
	const glm::vec3 voxelRTF = voxelPos + (voxelCornerForAxis(rect, axis) * voxelWidth) - radius;
	//const float uvScale = 1.f / (std::min)((std::min)(_sizeX, _sizeY), _sizeZ);

	const glm::vec3 positions[8] = {
		glm::vec3(voxelLBR.x, voxelLBR.y, voxelLBR.z), // 0 left-bottom-rear
		glm::vec3(voxelRTF.x, voxelLBR.y, voxelLBR.z), // 1 right-bottom-rear
		glm::vec3(voxelLBR.x, voxelRTF.y, voxelLBR.z), // 2 left-top-rear
		glm::vec3(voxelRTF.x, voxelRTF.y, voxelLBR.z), // 3 right-top-rear
		glm::vec3(voxelLBR.x, voxelLBR.y, voxelRTF.z), // 4 left-bottom-front
		glm::vec3(voxelRTF.x, voxelLBR.y, voxelRTF.z), // 5 right-bottom-front
		glm::vec3(voxelLBR.x, voxelRTF.y, voxelRTF.z), // 6 left-top-front
		glm::vec3(voxelRTF.x, voxelRTF.y, voxelRTF.z), // 7 right-top-front
	};

	const int startIndex = axis * 6;
	const int endIndex = startIndex + 6;
	for (int index = startIndex; index < endIndex; index++)
	{
		const int index_side = index / 6;
		const int index_vertex = CubeConstants::cube_indices[index];
		verts[vertCount++] = {
			positions[index_vertex],
			CubeConstants::cube_normals[index_side],
			//COLOR_WHITE,
			HSVColor(index * 10.f, 1.f, (float)index / 35.f),
			glm::vec3(1, 1, 1)
		};
	}
}

void VoxelData::createTriangleMeshReduced(VoxelMeshPBRVertexData* verts, size_t& vertexCount, const float radius, const glm::vec3& offset) const
{
	// Z-Faces
	for (int x = 0; x < _sizeX; x++)
	{
		std::vector<SurfaceRect> rectsForSliceBack;
		std::vector<SurfaceRect> rectsForSliceFront;
		for (int y = 0; y < _sizeY; y++)
		{
			uint8_t previousVoxelB = EMPTY_VOXEL;
			uint8_t previousVoxelF = EMPTY_VOXEL;
			bool previousBackFaceVisible = false;
			bool previousFrontFaceVisible = false;
			std::vector<SurfaceRect> rectsForRowBack;
			std::vector<SurfaceRect> rectsForRowFront;
			for (int z = 0; z < _sizeZ; z++)
			{
				const glm::ivec3 voxelCoord = glm::ivec3(x, y, z);
				//Log::Debug("Z: (%i, %i, %i) rects: %i, %i", x, y, z, rectsForRowBack.size(), rectsForRowFront.size());
				extractFaceRect(voxelCoord, glm::ivec3(x - 1, y, z), previousVoxelB, previousBackFaceVisible, rectsForRowBack, Axis::Z);
				extractFaceRect(voxelCoord, glm::ivec3(x + 1, y, z), previousVoxelF, previousFrontFaceVisible, rectsForRowFront, Axis::Z);
			}
			mergeRects(rectsForRowBack, rectsForSliceBack);
			mergeRects(rectsForRowFront, rectsForSliceFront);
			//Log::Debug("Y: (%i, %i) rects: %i, %i", x, y, rectsForRowBack.size(), rectsForRowFront.size());
		}
		for (const SurfaceRect& rect : rectsForSliceBack)
		{
			createFaceVerts(rect, x, 0, radius, offset, verts, vertexCount);
		}
		for (const SurfaceRect& rect : rectsForSliceFront)
		{
			createFaceVerts(rect, x, 1, radius, offset, verts, vertexCount);
		}
	}
	// Y Faces
	for (int y = 0; y < _sizeY; y++)
	{
		std::vector<SurfaceRect> rectsForSliceBack;
		std::vector<SurfaceRect> rectsForSliceFront;
		for (int z = 0; z < _sizeZ; z++)
		{
			uint8_t previousVoxelB = EMPTY_VOXEL;
			uint8_t previousVoxelF = EMPTY_VOXEL;
			bool previousBackFaceVisible = false;
			bool previousFrontFaceVisible = false;
			std::vector<SurfaceRect> rectsForRowBack;
			std::vector<SurfaceRect> rectsForRowFront;
			for (int x = 0; x < _sizeX; x++)
			{
				const glm::ivec3 voxelCoord = glm::ivec3(x, y, z);
				//Log::Debug("Z: (%i, %i, %i) rects: %i, %i", x, y, z, rectsForRowBack.size(), rectsForRowFront.size());
				extractFaceRect(voxelCoord, glm::ivec3(x, y - 1, z), previousVoxelB, previousBackFaceVisible, rectsForRowBack, Axis::Y);
				extractFaceRect(voxelCoord, glm::ivec3(x, y + 1, z), previousVoxelF, previousFrontFaceVisible, rectsForRowFront, Axis::Y);
			}
			mergeRects(rectsForRowBack, rectsForSliceBack);
			mergeRects(rectsForRowFront, rectsForSliceFront);
			//Log::Debug("X: (%i, %i) rects: %i, %i", x, y, rectsForRowBack.size(), rectsForRowFront.size());
		}
		for (const SurfaceRect& rect : rectsForSliceBack)
		{
			createFaceVerts(rect, y, 2, radius, offset, verts, vertexCount);
		}
		for (const SurfaceRect& rect : rectsForSliceFront)
		{
			createFaceVerts(rect, y, 3, radius, offset, verts, vertexCount);
		}
	}
	// X Faces
	for (int z = 0; z < _sizeZ; z++)
	{
		std::vector<SurfaceRect> rectsForSliceBack;
		std::vector<SurfaceRect> rectsForSliceFront;
		for (int y = 0; y < _sizeY; y++)
		{
			uint8_t previousVoxelB = EMPTY_VOXEL;
			uint8_t previousVoxelF = EMPTY_VOXEL;
			bool previousBackFaceVisible = false;
			bool previousFrontFaceVisible = false;
			std::vector<SurfaceRect> rectsForRowBack;
			std::vector<SurfaceRect> rectsForRowFront;
			for (int x = 0; x < _sizeX; x++)
			{
				const glm::ivec3 voxelCoord = glm::ivec3(x, y, z);
				extractFaceRect(voxelCoord, glm::ivec3(x, y, z - 1), previousVoxelB, previousBackFaceVisible, rectsForRowBack, Axis::X);
				extractFaceRect(voxelCoord, glm::ivec3(x, y, z + 1), previousVoxelF, previousFrontFaceVisible, rectsForRowFront, Axis::X);
			}
			mergeRects(rectsForRowBack, rectsForSliceBack);
			mergeRects(rectsForRowFront, rectsForSliceFront);
		}
		for (const SurfaceRect& rect : rectsForSliceBack)
		{
			createFaceVerts(rect, z, 4, radius, offset, verts, vertexCount);
		}
		for (const SurfaceRect& rect : rectsForSliceFront)
		{
			createFaceVerts(rect, z, 5, radius, offset, verts, vertexCount);
		}
	}
}

const uint32_t VoxelData::getPhysicsReduced(Physics& physics, float radius) const
{
	btTriangleMesh* triangleMesh = physics.createTriangleMesh();

	const float r2 = radius * 2.0f;
	const glm::vec3 voxelOffset = glm::vec3(radius - (_sizeX*radius), radius - (_sizeY*radius), radius - (_sizeZ*radius));
	const size_t numVoxels = _sizeX*_sizeY*_sizeZ;
	// Worst case is every voxel filled, no empty voxels
	// This results in 36 vertices per each voxel
	const size_t worst_case = 36 * numVoxels;
	//btVector3* newVerts = CUSTOM_NEW_ARRAY(btVector3, worst_case, m_allocator);
	//btVector3* newVerts = (btVector3*)m_allocator.allocate(sizeof(btVector3) * worst_case);
	btVector3* newVerts = new btVector3[worst_case];
	//Log::Debug("new verts at %p", newVerts);
	size_t numVerts = 0;

	int merged = 0;
	bool vis = false;

	// View from negative x
	for (int x = _sizeX - 1; x >= 0; x--) {
		for (int y = 0; y < _sizeY; y++) {
			for (int z = 0; z < _sizeZ; z++) {
				const int arrPos = getIndex(x, y, z);
				const uint8_t b = _data[arrPos];
				if (blocksPhysics(x - 1, y, z)) {
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				btVector3 lbr_p = btVector3(pos.x - radius, pos.y - radius, pos.z - radius);
				btVector3 lbf_p = btVector3(pos.x - radius, pos.y - radius, pos.z + radius);
				btVector3 ltr_p = btVector3(pos.x - radius, pos.y + radius, pos.z - radius);
				btVector3 ltf_p = btVector3(pos.x - radius, pos.y + radius, pos.z + radius);
				if (vis && z != 0) {                   // Same block as previous one? Extend it.
					newVerts[numVerts - 5] = lbf_p;
					newVerts[numVerts - 2] = lbf_p;
					newVerts[numVerts - 1] = ltf_p;
					merged++;
				}
				else {                                // Otherwise, add a new quad.
					newVerts[numVerts++] = lbr_p;
					newVerts[numVerts++] = lbf_p;
					newVerts[numVerts++] = ltr_p;
					newVerts[numVerts++] = ltr_p;
					newVerts[numVerts++] = lbf_p;
					newVerts[numVerts++] = ltf_p;
				}
				vis = true;
			}
		}
	}
	// View from positive x
	for (int x = 0; x < _sizeX; x++) {
		for (int y = 0; y < _sizeY; y++) {
			for (int z = 0; z < _sizeZ; z++) {
				const int arrPos = getIndex(x, y, z);
				const uint8_t b = _data[arrPos];
				if (blocksPhysics(x + 1, y, z)) {
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				btVector3 rbr_p = btVector3(pos.x + radius, pos.y - radius, pos.z - radius);
				btVector3 rbf_p = btVector3(pos.x + radius, pos.y - radius, pos.z + radius);
				btVector3 rtr_p = btVector3(pos.x + radius, pos.y + radius, pos.z - radius);
				btVector3 rtf_p = btVector3(pos.x + radius, pos.y + radius, pos.z + radius);
				if (vis && z != 0) {
					newVerts[numVerts - 4] = rbf_p;
					newVerts[numVerts - 2] = rtf_p;
					newVerts[numVerts - 1] = rbf_p;
					merged++;
				}
				else {
					newVerts[numVerts++] = rbr_p;
					newVerts[numVerts++] = rtr_p;
					newVerts[numVerts++] = rbf_p;
					newVerts[numVerts++] = rtr_p;
					newVerts[numVerts++] = rtf_p;
					newVerts[numVerts++] = rbf_p;
				}
				vis = true;
			}
		}
	}
	// View from negative y
	for (int x = 0; x < _sizeX; x++) {
		for (int y = _sizeY - 1; y >= 0; y--) {
			for (int z = 0; z < _sizeZ; z++) {
				const int arrPos = getIndex(x, y, z);
				const uint8_t b = _data[arrPos];
				if (blocksPhysics(x, y - 1, z)) {
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				btVector3 lbr_p = btVector3(pos.x - radius, pos.y - radius, pos.z - radius);
				btVector3 lbf_p = btVector3(pos.x - radius, pos.y - radius, pos.z + radius);
				btVector3 rbr_p = btVector3(pos.x + radius, pos.y - radius, pos.z - radius);
				btVector3 rbf_p = btVector3(pos.x + radius, pos.y - radius, pos.z + radius);
				if (vis && z != 0) {
					newVerts[numVerts - 4] = lbf_p;
					newVerts[numVerts - 2] = rbf_p;
					newVerts[numVerts - 1] = lbf_p;
					merged++;
				}
				else {
					newVerts[numVerts++] = lbr_p;
					newVerts[numVerts++] = rbr_p;
					newVerts[numVerts++] = lbf_p;
					newVerts[numVerts++] = rbr_p;
					newVerts[numVerts++] = rbf_p;
					newVerts[numVerts++] = lbf_p;
				}
				vis = true;
			}
		}
	}
	// View from positive y
	for (int x = 0; x < _sizeX; x++) {
		for (int y = 0; y < _sizeY; y++) {
			for (int z = 0; z < _sizeZ; z++) {
				const int arrPos = getIndex(x, y, z);
				const uint8_t b = _data[arrPos];
				if (blocksPhysics(x, y + 1, z)) {
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				btVector3 ltr_p = btVector3(pos.x - radius, pos.y + radius, pos.z - radius);
				btVector3 ltf_p = btVector3(pos.x - radius, pos.y + radius, pos.z + radius);
				btVector3 rtr_p = btVector3(pos.x + radius, pos.y + radius, pos.z - radius);
				btVector3 rtf_p = btVector3(pos.x + radius, pos.y + radius, pos.z + radius);
				if (vis && z != 0) {
					newVerts[numVerts - 5] = ltf_p;
					newVerts[numVerts - 2] = ltf_p;
					newVerts[numVerts - 1] = rtf_p;
					merged++;
				}
				else {
					newVerts[numVerts++] = ltr_p;
					newVerts[numVerts++] = ltf_p;
					newVerts[numVerts++] = rtr_p;
					newVerts[numVerts++] = rtr_p;
					newVerts[numVerts++] = ltf_p;
					newVerts[numVerts++] = rtf_p;
				}
				vis = true;
			}
		}
	}
	// View from negative z
	for (int x = 0; x < _sizeX; x++) {
		for (int z = _sizeZ - 1; z >= 0; z--) {
			for (int y = 0; y < _sizeY; y++) {
				const int arrPos = getIndex(x, y, z);
				const uint8_t b = _data[arrPos];
				if (blocksPhysics(x, y, z - 1)) {
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				btVector3 lbr_p = btVector3(pos.x - radius, pos.y - radius, pos.z - radius);
				btVector3 ltr_p = btVector3(pos.x - radius, pos.y + radius, pos.z - radius);
				btVector3 rbr_p = btVector3(pos.x + radius, pos.y - radius, pos.z - radius);
				btVector3 rtr_p = btVector3(pos.x + radius, pos.y + radius, pos.z - radius);
				if (vis && y != 0) {
					newVerts[numVerts - 5] = ltr_p;
					newVerts[numVerts - 3] = ltr_p;
					newVerts[numVerts - 2] = rtr_p;
					merged++;
				}
				else {
					newVerts[numVerts++] = lbr_p;
					newVerts[numVerts++] = ltr_p;
					newVerts[numVerts++] = rbr_p;
					newVerts[numVerts++] = ltr_p;
					newVerts[numVerts++] = rtr_p;
					newVerts[numVerts++] = rbr_p;
				}
				vis = true;
			}
		}
	}
	// View from positive z
	for (int x = 0; x < _sizeX; x++) {
		for (int z = 0; z < _sizeZ; z++) {
			for (int y = 0; y < _sizeY; y++) {
				const int arrPos = getIndex(x, y, z);
				const uint8_t b = _data[arrPos];
				if (blocksPhysics(x, y, z + 1)) {
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				btVector3 lbf_p = btVector3(pos.x - radius, pos.y - radius, pos.z + radius);
				btVector3 ltf_p = btVector3(pos.x - radius, pos.y + radius, pos.z + radius);
				btVector3 rbf_p = btVector3(pos.x + radius, pos.y - radius, pos.z + radius);
				btVector3 rtf_p = btVector3(pos.x + radius, pos.y + radius, pos.z + radius);
				if (vis && y != 0) {
					newVerts[numVerts - 4] = ltf_p;
					newVerts[numVerts - 3] = ltf_p;
					newVerts[numVerts - 1] = rtf_p;
					merged++;
				}
				else {
					newVerts[numVerts++] = lbf_p;
					newVerts[numVerts++] = rbf_p;
					newVerts[numVerts++] = ltf_p;
					newVerts[numVerts++] = ltf_p;
					newVerts[numVerts++] = rbf_p;
					newVerts[numVerts++] = rtf_p;
				}
				vis = true;
			}
		}
	}
	int numTris = numVerts / 3;
	for (int i = 0; i < numTris; i++)
	{
		// Copy triangle data into new physics mesh
		triangleMesh->addTriangle(
			newVerts[i * 3 + 0],
			newVerts[i * 3 + 1],
			newVerts[i * 3 + 2]);
	}

	//CUSTOM_DELETE_ARRAY(newVerts, m_allocator);
	//m_allocator.deallocate(newVerts);
	delete[] newVerts;

	const bool useQuantizedAABB = true;
	const uint32_t shapeID = physics.createTriangleMeshShape(triangleMesh, useQuantizedAABB);
	return shapeID;
}

const uint32_t VoxelData::getPhysicsCubes(Physics& physics, const float radius) const
{
	uint32_t shapeID = physics.createCompountShape();
	btCompoundShape* shape = (btCompoundShape*)physics.getShapeForID(shapeID);
	const uint32_t cubeShapeID = physics.createCube(radius);
	btBoxShape* cube = (btBoxShape*)physics.getShapeForID(cubeShapeID);
 
	const float r2 = radius * 2.0f;
	const glm::vec3 voxelOffset = glm::vec3(radius, radius, radius) - glm::vec3(_sizeX * radius, _sizeY * radius, _sizeZ * radius);
	const size_t numVoxels = _sizeX * _sizeY * _sizeZ;

	for (size_t i = 0; i < numVoxels; i++)
	{
		const uint8_t b = _data[i];

		if (b == EMPTY_VOXEL)
		{
			continue;
		}
		const glm::ivec3 coord = coordinateFromLinearIndex(i, _sizeX, _sizeY);
		glm::vec3 pos = (glm::vec3(coord.x, coord.y, coord.z)*r2) + voxelOffset;

		// Add triangles to mesh
		if (blocksPhysics(coord.x - 1, coord.y, coord.z) &&
			blocksPhysics(coord.x + 1, coord.y, coord.z) &&
			blocksPhysics(coord.x, coord.y - 1, coord.z) &&
			blocksPhysics(coord.x, coord.y + 1, coord.z) &&
			blocksPhysics(coord.x, coord.y, coord.z - 1) &&
			blocksPhysics(coord.x, coord.y, coord.z + 1)) {
			continue;
		}
		//btBoxShape* box = new btBoxShape(btVector3(radius, radius, radius));
		btTransform trans = btTransform();
		trans.setIdentity();
		trans.setOrigin(btVector3(pos.x, pos.y, pos.z));
		shape->addChildShape(trans, cube);
	}
	return shapeID;
}

const uint32_t VoxelData::getPhysicsHull(Physics& physics, const float radius) const
{
	uint32_t shapeID = physics.createConvexHullShape();
	btConvexHullShape* shape = (btConvexHullShape*)physics.getShapeForID(shapeID);

	float r2 = radius * 2.0f;
	glm::vec3 voxelOffset = glm::vec3(radius - (_sizeX*radius), radius - (_sizeY*radius), radius - (_sizeZ*radius));
	const int numVoxels = _sizeX*_sizeY*_sizeZ;

	for (int i = 0; i < numVoxels; i++)
	{
		const uint8_t b = _data[i];

		if (b == EMPTY_VOXEL)
		{
			continue;
		}
		const glm::ivec3 coord = coordinateFromLinearIndex(i, _sizeX, _sizeY);
		glm::vec3 pos = (glm::vec3(coord.x, coord.y, coord.z)*r2) + voxelOffset;

		// Add triangles to mesh
		if (blocksPhysics(coord.x - 1, coord.y, coord.z) &&
			blocksPhysics(coord.x + 1, coord.y, coord.z) &&
			blocksPhysics(coord.x, coord.y - 1, coord.z) &&
			blocksPhysics(coord.x, coord.y + 1, coord.z) &&
			blocksPhysics(coord.x, coord.y, coord.z - 1) &&
			blocksPhysics(coord.x, coord.y, coord.z + 1)) {
			continue;
		}
		btVector3 lbr_p = btVector3(pos.x - radius, pos.y - radius, pos.z - radius);
		btVector3 lbf_p = btVector3(pos.x - radius, pos.y - radius, pos.z + radius);
		btVector3 ltr_p = btVector3(pos.x - radius, pos.y + radius, pos.z - radius);
		btVector3 ltf_p = btVector3(pos.x - radius, pos.y + radius, pos.z + radius);
		btVector3 rbr_p = btVector3(pos.x + radius, pos.y - radius, pos.z - radius);
		btVector3 rbf_p = btVector3(pos.x + radius, pos.y - radius, pos.z + radius);
		btVector3 rtr_p = btVector3(pos.x + radius, pos.y + radius, pos.z - radius);
		btVector3 rtf_p = btVector3(pos.x + radius, pos.y + radius, pos.z + radius);
		shape->addPoint(lbr_p);
		shape->addPoint(lbf_p);
		shape->addPoint(ltr_p);
		shape->addPoint(ltf_p);
		shape->addPoint(rbr_p);
		shape->addPoint(rbf_p);
		shape->addPoint(rtr_p);
		shape->addPoint(rtf_p);
	}

	return shapeID;
}

const uint32_t VoxelData::getPhysicsAABBs(Physics& physics, const glm::vec3& radius) const
{
	uint32_t shapeID = physics.createCompountShape();
	btCompoundShape* shape = (btCompoundShape*)physics.getShapeForID(shapeID);

	std::vector<VoxelAABB> aabbVect;       // Final AABBs
	std::vector<VoxelAABB> aabbVectXPrev;  // Previous pass of X AABBs
	float minRadius = radius.x < radius.y ? radius.x : radius.y;
	minRadius = radius.z < minRadius ? radius.z : minRadius;

	float epsilon = minRadius*0.1f;
	float r2 = minRadius * 2.0f;
	glm::vec3 voxelOffset = glm::vec3(radius.x - (_sizeX*radius.x), radius.y - (_sizeY*radius.y), radius.z - (_sizeZ*radius.z));

	for (int px = 0; px < _sizeX; px++) 
	{
		std::vector<VoxelAABB> aabbVectX;      // This pass of X AABBs
		std::vector<VoxelAABB> aabbVectYPrev;  // Previous pass of Y AABBs
		for (int py = 0; py < _sizeY; py++)
		{
			std::vector<VoxelAABB> aabbVectY;  // This pass of Y AABBs
			uint8_t prevZ = EMPTY_VOXEL;
			for (int pz = 0; pz < _sizeZ; pz++) 
			{
				const int arrPos = getIndex(px, py, pz);
				const uint8_t b = _data[arrPos];

				if (b == EMPTY_VOXEL)
				{
					prevZ = EMPTY_VOXEL;
					continue;
				}
				const glm::vec3 pos = (glm::vec3(px, py, pz)*r2) + voxelOffset;

				if (prevZ == EMPTY_VOXEL)
				{
					VoxelAABB aabb = VoxelAABB(pos - glm::vec3(radius), pos + glm::vec3(radius), b);
					aabbVectY.push_back(aabb);
				}
				else 
				{ 
					// Same type, extend AABB
					aabbVectY.back().m_max.z = pos.z + radius.z;
				}

				prevZ = b;

				// Compare current new y aabbs with previous set
				for (int p = 0; p < aabbVectYPrev.size(); p++)
				{
					VoxelAABB& prev = aabbVectYPrev[p];
					if (fabsf(prev.m_min.x - aabbVectY.back().m_min.x) < epsilon &&
						fabsf(prev.m_max.x - aabbVectY.back().m_max.x) < epsilon &&
						fabsf(prev.m_min.z - aabbVectY.back().m_min.z) < epsilon &&
						fabsf(prev.m_max.z - aabbVectY.back().m_max.z) < epsilon &&
						fabsf(prev.m_max.y - aabbVectY.back().m_min.y) < epsilon)
					{
						aabbVectY.back().m_min.y = prev.m_min.y;
						aabbVectYPrev.erase(aabbVectYPrev.begin() + p);
						prevZ = EMPTY_VOXEL;    // Merged - must not merge with this one next
						break;
					}
				}
			}   // For each z
			if (aabbVectYPrev.size() != 0) {  // Copy previous Y aabbs to final vect
				std::copy(aabbVectYPrev.begin(), aabbVectYPrev.end(), std::back_inserter(aabbVectX));
				aabbVectYPrev.clear();
			}
			if (aabbVectY.size() != 0) {      // Copy previous Y aabbs to previous vect
				std::copy(aabbVectY.begin(), aabbVectY.end(), std::back_inserter(aabbVectYPrev));
				aabbVectY.clear();
			}
			if (py == _sizeY - 1) {         // Copy previous Y aabbs to final vect (in case left over)
				if (aabbVectYPrev.size() != 0) {
					std::copy(aabbVectYPrev.begin(), aabbVectYPrev.end(), std::back_inserter(aabbVectX));
					aabbVectYPrev.clear();
				}
			}
		}   // For each y


		// Compare new X aabbs with previous set
		for (int pXP = 0; pXP < aabbVectXPrev.size(); pXP++) 
		{
			if (aabbVectXPrev.empty() || pXP == aabbVectXPrev.size())
			{
				break;
			}
			for (int pX = 0; pX < aabbVectX.size(); pX++)
			{
				VoxelAABB& prev = aabbVectXPrev[pXP];
				VoxelAABB& aabb = aabbVectX[pX];
				if (fabsf(prev.m_min.y - aabb.m_min.y) < epsilon &&
					fabsf(prev.m_max.y - aabb.m_max.y) < epsilon &&
					fabsf(prev.m_min.z - aabb.m_min.z) < epsilon &&
					fabsf(prev.m_max.z - aabb.m_max.z) < epsilon &&
					fabsf(prev.m_max.x - aabb.m_min.x) < epsilon) {
					aabb.m_min.x = prev.m_min.x;
					aabbVectXPrev.erase(aabbVectXPrev.begin() + pXP);
					if (aabbVectXPrev.empty() || pXP == aabbVectXPrev.size())
					{
						break;
					}
				}
			}
		}
		if (aabbVectXPrev.size() != 0) 
		{
			// Copy previous X aabbs to final vect
			std::copy(aabbVectXPrev.begin(), aabbVectXPrev.end(), std::back_inserter(aabbVect));
			aabbVectXPrev.clear();
		}
		if (aabbVectX.size() != 0)
		{
			// Copy current X aabbs to previous vect
			std::copy(aabbVectX.begin(), aabbVectX.end(), std::back_inserter(aabbVectXPrev));
			aabbVectX.clear();
		}
		if (px == _sizeX - 1)
		{
			if (aabbVectXPrev.size() != 0)
			{ 
				// Copy previous X aabbs to final vect (in case left over)
				std::copy(aabbVectXPrev.begin(), aabbVectXPrev.end(), std::back_inserter(aabbVect));
				aabbVectXPrev.clear();
			}
		}
	}   // For each x

	// Create physics shapes from AABBs
	Log::Debug("[VoxelData] meshed %i physics AABBs", aabbVect.size());
	for (const VoxelAABB& aabb : aabbVect)
	{
		const glm::vec3 aaBBSize_2 = (aabb.m_max - aabb.m_min) * 0.5f;
		const glm::vec3 pos = aabb.m_min + aaBBSize_2;
		const uint32_t boxID = physics.createBox(aaBBSize_2.x, aaBBSize_2.y, aaBBSize_2.z);
		btBoxShape* box = (btBoxShape*)physics.getShapeForID(boxID);
		btTransform trans = btTransform();
		trans.setIdentity();
		trans.setOrigin(btVector3(pos.x, pos.y, pos.z));
		shape->addChildShape(trans, box);
	}

	return shapeID;
}
