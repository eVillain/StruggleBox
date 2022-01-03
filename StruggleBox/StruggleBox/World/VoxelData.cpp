#include "VoxelData.h"

#include "Allocator.h"
#include "ArenaOperators.h"
#include "Mesh.h"
#include "GFXDefines.h"
#include "MaterialData.h"
#include "Serialise.h"
#include "CubeConstants.h"
#include "Log.h"
#include "Physics.h"
#include "VoxelAABB.h"

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

void VoxelData::getMeshLinear(Mesh& mesh, float radius)
{
	float voxelWidth = radius * 2.0f;
	const glm::vec3 size = glm::vec3(_sizeX, _sizeY, _sizeZ);
	const glm::vec3 voxelOffset = glm::vec3(radius) - (size * radius);
	const int numVoxels = _sizeX *_sizeY * _sizeZ;

	// Worst case is every voxel filled, no empty voxels
	// This results in 36 vertices per each voxel
	const int worst_case = 36 * numVoxels;
	MeshVertexData* newVerts = CUSTOM_NEW_ARRAY(MeshVertexData, worst_case, m_allocator);
	int numVerts = 0;

	for (int i = 0; i < numVoxels; i++)
	{
		const uint8_t& b = _data[i];
		if (b == EMPTY_VOXEL)
		{
			continue;
		}
		const glm::ivec3 coord = coordinateFromLinearIndex(i, _sizeX, _sizeY);
		glm::vec3 pos = (glm::vec3(coord.x, coord.y, coord.z) * voxelWidth) + voxelOffset;

		// Check visibility of each side
		bool visibility_x_neg = !blocksVisibility(coord.x - 1, coord.y, coord.z);
		bool visibility_x_pos = !blocksVisibility(coord.x + 1, coord.y, coord.z);
		bool visibility_y_neg = !blocksVisibility(coord.x, coord.y - 1, coord.z);
		bool visibility_y_pos = !blocksVisibility(coord.x, coord.y + 1, coord.z);
		bool visibility_z_neg = !blocksVisibility(coord.x, coord.y, coord.z - 1);
		bool visibility_z_pos = !blocksVisibility(coord.x, coord.y, coord.z + 1);
		bool visibility[6] = { visibility_x_neg, visibility_x_pos, visibility_y_neg, visibility_y_pos, visibility_z_neg, visibility_z_pos };

		if (!visibility_x_neg && !visibility_x_pos &&
			!visibility_y_neg && !visibility_y_pos &&
			!visibility_z_neg && !visibility_z_pos)
		{
			continue;
		}

		const glm::vec4 positions[8] = {
			glm::vec4(pos.x - radius, pos.y - radius, pos.z - radius, 1.f), // 0 left-bottom-rear
			glm::vec4(pos.x + radius, pos.y - radius, pos.z - radius, 1.f), // 1 right-bottom-rear
			glm::vec4(pos.x - radius, pos.y + radius, pos.z - radius, 1.f), // 2 left-top-rear
			glm::vec4(pos.x + radius, pos.y + radius, pos.z - radius, 1.f), // 3 right-top-rear
			glm::vec4(pos.x - radius, pos.y - radius, pos.z + radius, 1.f), // 4 left-bottom-front
			glm::vec4(pos.x + radius, pos.y - radius, pos.z + radius, 1.f), // 5 right-bottom-front
			glm::vec4(pos.x - radius, pos.y + radius, pos.z + radius, 1.f), // 6 left-top-front
			glm::vec4(pos.x + radius, pos.y + radius, pos.z + radius, 1.f), // 7 right-top-front
		};

		const glm::vec2 materialOffset = MaterialData::texOffset(b);
		const glm::vec3 voxelUV3D = glm::vec3(coord.x / (float)_sizeX, coord.y / (float)_sizeY, coord.z / (float)_sizeZ);
		for (int index = 0; index < 36; index++)
		{
			const int index_side = index / 6;
			if (!visibility[index_side])
			{
				continue;
			}

			glm::vec2 uvOffset;
			if (index_side < 2)
			{
				uvOffset = glm::vec2(voxelUV3D.z, voxelUV3D.y);
				if (index_side % 2)
				{
					uvOffset.x = 1.f - (uvOffset.x + (1.f / std::max(_sizeX, _sizeZ)));
				}
			}
			else if (index_side < 4)
			{
				uvOffset = glm::vec2(voxelUV3D.x, voxelUV3D.z);
				if (index_side % 2)
				{
					uvOffset.x = 1.f - (uvOffset.x + (1.f / std::max(_sizeX, _sizeZ)));
				}
			}
			else
			{
				uvOffset = glm::vec2(voxelUV3D.x, voxelUV3D.y);
				if (index_side % 2 == 0)
				{
					uvOffset.x = 1.f - (uvOffset.x + (1.f / std::max(_sizeX, _sizeZ)));
				}
			}

			const float uvScale = 1.f /std::min(std::min(_sizeX, _sizeY), _sizeZ);
			const glm::vec2 uv = (CubeConstants::cube_uvs[index] * uvScale) + uvOffset;
			const int index_vertex = CubeConstants::cube_indices[index];
			newVerts[numVerts++] = {
				positions[index_vertex].x, positions[index_vertex].y, positions[index_vertex].z, positions[index_vertex].w,
				CubeConstants::cube_normals[index_side].x, CubeConstants::cube_normals[index_side].y, CubeConstants::cube_normals[index_side].z,
				CubeConstants::cube_tangents[index_side].x, CubeConstants::cube_tangents[index_side].y, CubeConstants::cube_tangents[index_side].z,
				uv.x * _scale, uv.y* _scale,
				materialOffset.x, materialOffset.y
			};
		}
	}
	mesh.resize(numVerts);
	mesh.buffer(newVerts, numVerts);

	CUSTOM_DELETE_ARRAY(newVerts, m_allocator);
}

void VoxelData::getMeshLinearWithVertexAO(Mesh& mesh, float radius)
{
	float voxelWidth = radius * 2.0f;
	glm::vec3 voxelOffset = glm::vec3(radius) - (glm::vec3(_sizeX, _sizeY, _sizeZ) * radius);
	const int numVoxels = _sizeX * _sizeY * _sizeZ;

	// Worst case is every voxel filled, no empty voxels
	// This results in 36 vertices per each voxel
	const int worst_case = 36 * numVoxels;
	MeshVertexData* newVerts = CUSTOM_NEW_ARRAY(MeshVertexData, worst_case, m_allocator);
	int numVerts = 0;

	for (int i = 0; i < numVoxels; i++)
	{
		const uint8_t& b = _data[i];
		if (b == EMPTY_VOXEL)
		{
			continue;
		}
		const glm::ivec3 coord = coordinateFromLinearIndex(i, _sizeX, _sizeY);
		glm::vec3 pos = (glm::vec3(coord.x, coord.y, coord.z) * voxelWidth) + voxelOffset;

		// Check visibility of each side
		bool visibility_x_neg = !blocksVisibility(coord.x - 1, coord.y, coord.z);
		bool visibility_x_pos = !blocksVisibility(coord.x + 1, coord.y, coord.z);
		bool visibility_y_neg = !blocksVisibility(coord.x, coord.y - 1, coord.z);
		bool visibility_y_pos = !blocksVisibility(coord.x, coord.y + 1, coord.z);
		bool visibility_z_neg = !blocksVisibility(coord.x, coord.y, coord.z - 1);
		bool visibility_z_pos = !blocksVisibility(coord.x, coord.y, coord.z + 1);
		bool visibility[6] = { visibility_x_neg, visibility_x_pos, visibility_y_neg, visibility_y_pos, visibility_z_neg, visibility_z_pos };

		if (!visibility_x_neg && !visibility_x_pos &&
			!visibility_y_neg && !visibility_y_pos &&
			!visibility_z_neg && !visibility_z_pos)
		{
			continue;
		}
		GLfloat occlusion = 0.25f;
		// Each vertex has 4 neighbors to be checked for ambient occlusion
		// These are the amounts for each neighbor node
		GLfloat lbao = (blocksVisibility(coord.x - 1, coord.y - 1, coord.z)) ? occlusion : 0.0f;
		GLfloat ltao = (blocksVisibility(coord.x - 1, coord.y + 1, coord.z)) ? occlusion : 0.0f;
		GLfloat rbao = (blocksVisibility(coord.x + 1, coord.y - 1, coord.z)) ? occlusion : 0.0f;
		GLfloat rtao = (blocksVisibility(coord.x + 1, coord.y + 1, coord.z)) ? occlusion : 0.0f;
		GLfloat brao = (blocksVisibility(coord.x, coord.y - 1, coord.z - 1)) ? occlusion : 0.0f;
		GLfloat trao = (blocksVisibility(coord.x, coord.y + 1, coord.z - 1)) ? occlusion : 0.0f;
		GLfloat bfao = (blocksVisibility(coord.x, coord.y - 1, coord.z + 1)) ? occlusion : 0.0f;
		GLfloat tfao = (blocksVisibility(coord.x, coord.y + 1, coord.z + 1)) ? occlusion : 0.0f;
		GLfloat lrao = (blocksVisibility(coord.x - 1, coord.y, coord.z - 1)) ? occlusion : 0.0f;
		GLfloat rrao = (blocksVisibility(coord.x + 1, coord.y, coord.z - 1)) ? occlusion : 0.0f;
		GLfloat lfao = (blocksVisibility(coord.x - 1, coord.y, coord.z + 1)) ? occlusion : 0.0f;
		GLfloat rfao = (blocksVisibility(coord.x + 1, coord.y, coord.z + 1)) ? occlusion : 0.0f;
		GLfloat lbrao = (blocksVisibility(coord.x - 1, coord.y - 1, coord.z - 1)) ? occlusion : 0.0f;
		GLfloat lbfao = (blocksVisibility(coord.x - 1, coord.y - 1, coord.z + 1)) ? occlusion : 0.0f;
		GLfloat ltrao = (blocksVisibility(coord.x - 1, coord.y + 1, coord.z - 1)) ? occlusion : 0.0f;
		GLfloat ltfao = (blocksVisibility(coord.x - 1, coord.y + 1, coord.z + 1)) ? occlusion : 0.0f;
		GLfloat rbrao = (blocksVisibility(coord.x + 1, coord.y - 1, coord.z - 1)) ? occlusion : 0.0f;
		GLfloat rbfao = (blocksVisibility(coord.x + 1, coord.y - 1, coord.z + 1)) ? occlusion : 0.0f;
		GLfloat rtrao = (blocksVisibility(coord.x + 1, coord.y + 1, coord.z - 1)) ? occlusion : 0.0f;
		GLfloat rtfao = (blocksVisibility(coord.x + 1, coord.y + 1, coord.z + 1)) ? occlusion : 0.0f;

		const glm::vec4 positions[8] = {
			glm::vec4(pos.x - radius, pos.y - radius, pos.z - radius, occlusion + lbao + lrao + lbrao), // 0 left-bottom-rear
			glm::vec4(pos.x + radius, pos.y - radius, pos.z - radius, occlusion + rbao + rrao + rbrao), // 1 right-bottom-rear
			glm::vec4(pos.x - radius, pos.y + radius, pos.z - radius, occlusion + ltao + lrao + ltrao), // 2 left-top-rear
			glm::vec4(pos.x + radius, pos.y + radius, pos.z - radius, occlusion + rtao + rrao + rtrao), // 3 right-top-rear
			glm::vec4(pos.x - radius, pos.y - radius, pos.z + radius, occlusion + lbao + lfao + lbfao), // 4 left-bottom-front
			glm::vec4(pos.x + radius, pos.y - radius, pos.z + radius, occlusion + rbao + rfao + rbfao), // 5 right-bottom-front
			glm::vec4(pos.x - radius, pos.y + radius, pos.z + radius, occlusion + ltao + lfao + ltfao), // 6 left-top-front
			glm::vec4(pos.x + radius, pos.y + radius, pos.z + radius, occlusion + rtao + rfao + rtfao), // 7 right-top-front
		};

		const glm::vec2 materialOffset = MaterialData::texOffset(b);

		for (int index = 0; index < 36; index++)
		{
			const int index_side = index / 6;
			if (!visibility[index_side])
			{
				continue;
			}
			const int index_vertex = CubeConstants::cube_indices[index];
			newVerts[numVerts++] = {
				positions[index_vertex].x, positions[index_vertex].y, positions[index_vertex].z, positions[index_vertex].w,
				CubeConstants::cube_normals[index_side].x, CubeConstants::cube_normals[index_side].y, CubeConstants::cube_normals[index_side].z,
				CubeConstants::cube_tangents[index_side].x, CubeConstants::cube_tangents[index_side].y, CubeConstants::cube_tangents[index_side].z,
				CubeConstants::cube_uvs[index].x, CubeConstants::cube_uvs[index].y,
				materialOffset.x, materialOffset.y
			};
		}
	}
	mesh.resize(numVerts);
	mesh.buffer(newVerts, numVerts);
	m_allocator.deallocate(newVerts);
}

void VoxelData::getMeshReduced(Mesh& mesh, float radius)
{
	float r2 = radius * 2.0f;
	glm::vec3 voxelOffset = glm::vec3(radius - (_sizeX*radius), radius - (_sizeY*radius), radius - (_sizeZ*radius));
	const int numVoxels = _sizeX*_sizeY*_sizeZ;
	// Worst case is every other voxel filled
	// This results in 36 vertices per each voxel for 1/2 of all voxels = 18
	const int worst_case = 18 * numVoxels;
	MeshVertexData* newVerts = CUSTOM_NEW_ARRAY(MeshVertexData, worst_case, m_allocator);
	int numVerts = 0;

	int merged = 0;
	bool vis = false;;
	const GLfloat occlusion = 0.25f;
	const glm::vec3 normals[6] = {
		glm::vec3(-1,0,0),
		glm::vec3(1,0,0),
		glm::vec3(0,-1,0),
		glm::vec3(0,1,0),
		glm::vec3(0,0,-1),
		glm::vec3(0,0,1),
	};
	// View from negative x
	for (int x = _sizeX - 1; x >= 0; x--)
	{
		for (int y = 0; y < _sizeY; y++)
		{
			for (int z = 0; z < _sizeZ; z++)
			{
				uint8_t& b = _data[linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY)];
				if (b == EMPTY_VOXEL ||
					blocksPhysics(x - 1, y, z))
				{
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				const glm::vec2 materialOffset = MaterialData::texOffset(b);

				// Each vertex has 4 neighbors to be checked for ambient occlusion
				// These are the amounts for each neighbor node
				GLfloat lbao = (blocksVisibility(x - 1, y - 1, z)) ? occlusion : 0.0f;
				GLfloat ltao = (blocksVisibility(x - 1, y + 1, z)) ? occlusion : 0.0f;
				GLfloat lrao = (blocksVisibility(x - 1, y, z - 1)) ? occlusion : 0.0f;
				GLfloat lfao = (blocksVisibility(x - 1, y, z + 1)) ? occlusion : 0.0f;
				GLfloat lbrao = (blocksVisibility(x - 1, y - 1, z - 1)) ? occlusion : 0.0f;
				GLfloat lbfao = (blocksVisibility(x - 1, y - 1, z + 1)) ? occlusion : 0.0f;
				GLfloat ltrao = (blocksVisibility(x - 1, y + 1, z - 1)) ? occlusion : 0.0f;
				GLfloat ltfao = (blocksVisibility(x - 1, y + 1, z + 1)) ? occlusion : 0.0f;

				MeshVertexData lbr = { pos.x - radius, pos.y - radius, pos.z - radius, occlusion + lbao + lrao + lbrao,
					normals[0].x, normals[0].y, normals[0].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData lbf = { pos.x - radius, pos.y - radius, pos.z + radius, occlusion + lbao + lfao + lbfao,
					normals[0].x, normals[0].y, normals[0].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData ltr = { pos.x - radius, pos.y + radius, pos.z - radius, occlusion + ltao + lrao + ltrao,
					normals[0].x, normals[0].y, normals[0].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData ltf = { pos.x - radius, pos.y + radius, pos.z + radius, occlusion + ltao + lfao + ltfao,
					normals[0].x, normals[0].y, normals[0].z, 1.0f, materialOffset.x, materialOffset.y
				};
				bool potentialMerge = false;
				if (vis && z != 0)
				{	
					uint8_t& b2 = _data[linearIndexFromCoordinate(x, y, z-1, _sizeX, _sizeY)];
					if (b == b2 &&
						newVerts[numVerts - 6].w == lbr.w &&
						newVerts[numVerts - 5].w == lbf.w &&
						newVerts[numVerts - 4].w == ltr.w &&
						newVerts[numVerts - 1].w == ltf.w)
						potentialMerge = true;
				}
				if (potentialMerge)
				{	// Same block as previous one and same occlusion? Extend it.
					newVerts[numVerts - 5] = lbf;
					newVerts[numVerts - 2] = lbf;
					newVerts[numVerts - 1] = ltf;
					merged++;
				}
				else
				{	// Otherwise, add a new quad.
					newVerts[numVerts++] = lbr;
					newVerts[numVerts++] = lbf;
					newVerts[numVerts++] = ltr;
					newVerts[numVerts++] = ltr;
					newVerts[numVerts++] = lbf;
					newVerts[numVerts++] = ltf;
				}
				vis = true;
			}
		}
	}
	vis = false;

	// View from positive x
	for (int x = 0; x < _sizeX; x++)
	{
		for (int y = 0; y < _sizeY; y++)
		{
			for (int z = 0; z < _sizeZ; z++)
			{
				uint8_t& b = _data[linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY)];
				if (b == EMPTY_VOXEL ||
					blocksPhysics(x + 1, y, z))
				{
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				const glm::vec2 materialOffset = MaterialData::texOffset(b);
				GLfloat rbao = (blocksVisibility(x + 1, y - 1, z)) ? occlusion : 0.0f;
				GLfloat rtao = (blocksVisibility(x + 1, y + 1, z)) ? occlusion : 0.0f;
				GLfloat rrao = (blocksVisibility(x + 1, y, z - 1)) ? occlusion : 0.0f;
				GLfloat rfao = (blocksVisibility(x + 1, y, z + 1)) ? occlusion : 0.0f;
				GLfloat rbrao = (blocksVisibility(x + 1, y - 1, z - 1)) ? occlusion : 0.0f;
				GLfloat rbfao = (blocksVisibility(x + 1, y - 1, z + 1)) ? occlusion : 0.0f;
				GLfloat rtrao = (blocksVisibility(x + 1, y + 1, z - 1)) ? occlusion : 0.0f;
				GLfloat rtfao = (blocksVisibility(x + 1, y + 1, z + 1)) ? occlusion : 0.0f;
				MeshVertexData rbr = { pos.x + radius, pos.y - radius, pos.z - radius, occlusion + rbao + rrao + rbrao,
					normals[1].x, normals[1].y, normals[1].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rbf = { pos.x + radius, pos.y - radius, pos.z + radius, occlusion + rbao + rfao + rbfao,
					normals[1].x, normals[1].y, normals[1].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rtr = { pos.x + radius, pos.y + radius, pos.z - radius, occlusion + rtao + rrao + rtrao,
					normals[1].x, normals[1].y, normals[1].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rtf = { pos.x + radius, pos.y + radius, pos.z + radius, occlusion + rtao + rfao + rtfao,
					normals[1].x, normals[1].y, normals[1].z, 1.0f, materialOffset.x, materialOffset.y
				};
				bool potentialMerge = false;
				if (vis && z != 0)
				{
					uint8_t& b2 = _data[linearIndexFromCoordinate(x, y, z - 1, _sizeX, _sizeY)];
					if (b == b2 &&
						newVerts[numVerts - 6].w == rbr.w &&
						newVerts[numVerts - 5].w == rtr.w &&
						newVerts[numVerts - 2].w == rtf.w &&
						newVerts[numVerts - 1].w == rbf.w)
						potentialMerge = true;
				}
				if (potentialMerge)
				{
					newVerts[numVerts - 4] = rbf;
					newVerts[numVerts - 2] = rtf;
					newVerts[numVerts - 1] = rbf;
					merged++;
				}
				else
				{
					newVerts[numVerts++] = rbr;
					newVerts[numVerts++] = rtr;
					newVerts[numVerts++] = rbf;
					newVerts[numVerts++] = rtr;
					newVerts[numVerts++] = rtf;
					newVerts[numVerts++] = rbf;
				}
				vis = true;
			}
		}
	}
	vis = false;

	// View from negative y
	for (int x = 0; x < _sizeX; x++) {
		for (int y = _sizeY - 1; y >= 0; y--) {
			for (int z = 0; z < _sizeZ; z++) {
				uint8_t& b = _data[linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY)];
				if (b == EMPTY_VOXEL ||
					blocksPhysics(x, y-1, z))
				{
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				const glm::vec2 materialOffset = MaterialData::texOffset(b);
				GLfloat brao = (blocksVisibility(x, y - 1, z - 1)) ? occlusion : 0.0f;
				GLfloat bfao = (blocksVisibility(x, y - 1, z + 1)) ? occlusion : 0.0f;
				GLfloat lbrao = (blocksVisibility(x - 1, y - 1, z - 1)) ? occlusion : 0.0f;
				GLfloat lbfao = (blocksVisibility(x - 1, y - 1, z + 1)) ? occlusion : 0.0f;
				GLfloat rbrao = (blocksVisibility(x + 1, y - 1, z - 1)) ? occlusion : 0.0f;
				GLfloat rbfao = (blocksVisibility(x + 1, y - 1, z + 1)) ? occlusion : 0.0f;
				MeshVertexData lbr = { pos.x - radius, pos.y - radius, pos.z - radius, occlusion + brao + brao + lbrao,
					normals[2].x, normals[2].y, normals[2].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData lbf = { pos.x - radius, pos.y - radius, pos.z + radius, occlusion + bfao + bfao + lbfao,
					normals[2].x, normals[2].y, normals[2].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rbr = { pos.x + radius, pos.y - radius, pos.z - radius, occlusion + brao + brao + rbrao,
					normals[2].x, normals[2].y, normals[2].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rbf = { pos.x + radius, pos.y - radius, pos.z + radius, occlusion + bfao + bfao + rbfao,
					normals[2].x, normals[2].y, normals[2].z, 1.0f, materialOffset.x, materialOffset.y
				};
				bool potentialMerge = false;
				if (vis && z != 0)
				{
					uint8_t& b2 = _data[linearIndexFromCoordinate(x, y, z - 1, _sizeX, _sizeY)];
					if (b == b2 &&
						newVerts[numVerts - 6].w == lbr.w &&
						newVerts[numVerts - 5].w == rbr.w &&
						newVerts[numVerts - 2].w == rbf.w &&
						newVerts[numVerts - 1].w == lbf.w) {
						potentialMerge = true;
					}
				}
				if (potentialMerge)
				{
					newVerts[numVerts - 4] = lbf;
					newVerts[numVerts - 2] = rbf;
					newVerts[numVerts - 1] = lbf;
					merged++;
				}
				else
				{
					newVerts[numVerts++] = lbr;
					newVerts[numVerts++] = rbr;
					newVerts[numVerts++] = lbf;
					newVerts[numVerts++] = rbr;
					newVerts[numVerts++] = rbf;
					newVerts[numVerts++] = lbf;
				}
				vis = true;
			}
		}
	}
	vis = false;

	// View from positive y
	for (int x = 0; x < _sizeX; x++) {
		for (int y = 0; y < _sizeY; y++) {
			for (int z = 0; z < _sizeZ; z++) {
				uint8_t& b = _data[linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY)];
				if (b == EMPTY_VOXEL ||
					blocksPhysics(x, y+1, z))
				{
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				const glm::vec2 materialOffset = MaterialData::texOffset(b);
				GLfloat ltao = (blocksVisibility(x - 1, y + 1, z)) ? occlusion : 0.0f;
				GLfloat rtao = (blocksVisibility(x + 1, y + 1, z)) ? occlusion : 0.0f;
				GLfloat trao = (blocksVisibility(x, y + 1, z - 1)) ? occlusion : 0.0f;
				GLfloat tfao = (blocksVisibility(x, y + 1, z + 1)) ? occlusion : 0.0f;
				GLfloat ltrao = (blocksVisibility(x - 1, y + 1, z - 1)) ? occlusion : 0.0f;
				GLfloat ltfao = (blocksVisibility(x - 1, y + 1, z + 1)) ? occlusion : 0.0f;
				GLfloat rtrao = (blocksVisibility(x + 1, y + 1, z - 1)) ? occlusion : 0.0f;
				GLfloat rtfao = (blocksVisibility(x + 1, y + 1, z + 1)) ? occlusion : 0.0f;
				MeshVertexData ltr = { pos.x - radius, pos.y + radius, pos.z - radius, occlusion + ltao + trao + ltrao,
					normals[3].x, normals[3].y, normals[3].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData ltf = { pos.x - radius, pos.y + radius, pos.z + radius, occlusion + ltao + tfao + ltfao,
					normals[3].x, normals[3].y, normals[3].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rtr = { pos.x + radius, pos.y + radius, pos.z - radius, occlusion + rtao + trao + rtrao,
					normals[3].x, normals[3].y, normals[3].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rtf = { pos.x + radius, pos.y + radius, pos.z + radius, occlusion + rtao + tfao + rtfao,
					normals[3].x, normals[3].y, normals[3].z, 1.0f, materialOffset.x, materialOffset.y
				};
				bool potentialMerge = false;
				if (vis && z != 0)
				{
					uint8_t& b2 = _data[linearIndexFromCoordinate(x, y, z - 1, _sizeX, _sizeY)];
					if (b == b2 &&
						newVerts[numVerts - 6].w == ltr.w &&
						newVerts[numVerts - 4].w == rtr.w &&
						newVerts[numVerts - 2].w == ltf.w &&
						newVerts[numVerts - 1].w == rtf.w)
							potentialMerge = true;
				}
				if (potentialMerge)
				{
					newVerts[numVerts - 5] = ltf;
					newVerts[numVerts - 2] = ltf;
					newVerts[numVerts - 1] = rtf;
					merged++;
				}
				else
				{
					newVerts[numVerts++] = ltr;
					newVerts[numVerts++] = ltf;
					newVerts[numVerts++] = rtr;
					newVerts[numVerts++] = rtr;
					newVerts[numVerts++] = ltf;
					newVerts[numVerts++] = rtf;
				}
				vis = true;
			}
		}
	}
	vis = false;

	// View from negative z
	for (int x = 0; x < _sizeX; x++)
	{
		for (int z = _sizeZ - 1; z >= 0; z--)
		{
			for (int y = 0; y < _sizeY; y++)
			{
				uint8_t& b = _data[linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY)];
				if (b == EMPTY_VOXEL ||
					blocksPhysics(x, y, z-1))
				{
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				const glm::vec2 materialOffset = MaterialData::texOffset(b);
				GLfloat brao = (blocksVisibility(x, y - 1, z - 1)) ? occlusion : 0.0f;
				GLfloat trao = (blocksVisibility(x, y + 1, z - 1)) ? occlusion : 0.0f;
				GLfloat lrao = (blocksVisibility(x - 1, y, z - 1)) ? occlusion : 0.0f;
				GLfloat rrao = (blocksVisibility(x + 1, y, z - 1)) ? occlusion : 0.0f;
				GLfloat lbrao = (blocksVisibility(x - 1, y - 1, z - 1)) ? occlusion : 0.0f;
				GLfloat ltrao = (blocksVisibility(x - 1, y + 1, z - 1)) ? occlusion : 0.0f;
				GLfloat rbrao = (blocksVisibility(x + 1, y - 1, z - 1)) ? occlusion : 0.0f;
				GLfloat rtrao = (blocksVisibility(x + 1, y + 1, z - 1)) ? occlusion : 0.0f;
				MeshVertexData lbr = { pos.x - radius, pos.y - radius, pos.z - radius, occlusion + brao + lrao + lbrao,
					normals[4].x, normals[4].y, normals[4].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData ltr = { pos.x - radius, pos.y + radius, pos.z - radius, occlusion + trao + lrao + ltrao,
					normals[4].x, normals[4].y, normals[4].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rbr = { pos.x + radius, pos.y - radius, pos.z - radius, occlusion + brao + rrao + rbrao,
					normals[4].x, normals[4].y, normals[4].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rtr = { pos.x + radius, pos.y + radius, pos.z - radius, occlusion + trao + rrao + rtrao,
					normals[4].x, normals[4].y, normals[4].z, 1.0f, materialOffset.x, materialOffset.y
				};
				bool potentialMerge = false;
				if (vis && y != 0) 
				{
					uint8_t& b2 = _data[linearIndexFromCoordinate(x, y-1, z, _sizeX, _sizeY)];
					if (b == b2 &&
						newVerts[numVerts - 6].w == lbr.w &&
						newVerts[numVerts - 5].w == ltr.w &&
						newVerts[numVerts - 2].w == rtr.w &&
						newVerts[numVerts - 1].w == rbr.w)
							potentialMerge = true;
				}
				if (potentialMerge)
				{
					newVerts[numVerts - 5] = ltr;
					newVerts[numVerts - 3] = ltr;
					newVerts[numVerts - 2] = rtr;
					merged++;
				}
				else
				{
					newVerts[numVerts++] = lbr;
					newVerts[numVerts++] = ltr;
					newVerts[numVerts++] = rbr;
					newVerts[numVerts++] = ltr;
					newVerts[numVerts++] = rtr;
					newVerts[numVerts++] = rbr;
				}
				vis = true;
			}
		}
	}
	vis = false;
	// View from positive z
	for (int x = 0; x < _sizeX; x++)
	{
		for (int z = 0; z < _sizeZ; z++)
		{
			for (int y = 0; y < _sizeY; y++)
			{
				uint8_t& b = _data[linearIndexFromCoordinate(x, y, z, _sizeX, _sizeY)];
				if (b == EMPTY_VOXEL ||
					blocksPhysics(x, y, z+1))
				{
					vis = false;
					continue;
				}
				glm::vec3 pos = (glm::vec3(x, y, z)*r2) + voxelOffset;
				const glm::vec2 materialOffset = MaterialData::texOffset(b);

				GLfloat bfao = (blocksVisibility(x, y - 1, z + 1)) ? occlusion : 0.0f;
				GLfloat tfao = (blocksVisibility(x, y + 1, z + 1)) ? occlusion : 0.0f;
				GLfloat lfao = (blocksVisibility(x - 1, y, z + 1)) ? occlusion : 0.0f;
				GLfloat rfao = (blocksVisibility(x + 1, y, z + 1)) ? occlusion : 0.0f;
				GLfloat lbfao = (blocksVisibility(x - 1, y - 1, z + 1)) ? occlusion : 0.0f;
				GLfloat ltfao = (blocksVisibility(x - 1, y + 1, z + 1)) ? occlusion : 0.0f;
				GLfloat rbfao = (blocksVisibility(x + 1, y - 1, z + 1)) ? occlusion : 0.0f;
				GLfloat rtfao = (blocksVisibility(x + 1, y + 1, z + 1)) ? occlusion : 0.0f;
				MeshVertexData lbf = { pos.x - radius, pos.y - radius, pos.z + radius, occlusion + bfao + lfao + lbfao,
					normals[5].x, normals[5].y, normals[5].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData ltf = { pos.x - radius, pos.y + radius, pos.z + radius, occlusion + tfao + lfao + ltfao,
					normals[5].x, normals[5].y, normals[5].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rbf = { pos.x + radius, pos.y - radius, pos.z + radius, occlusion + bfao + rfao + rbfao,
					normals[5].x, normals[5].y, normals[5].z, 1.0f, materialOffset.x, materialOffset.y
				};
				MeshVertexData rtf = { pos.x + radius, pos.y + radius, pos.z + radius, occlusion + tfao + rfao + rtfao,
					normals[5].x, normals[5].y, normals[5].z, 1.0f, materialOffset.x, materialOffset.y
				};
				bool potentialMerge = false;
				if (vis && y != 0)
				{  // Same block as previous one? Extend it.
					uint8_t& b2 = _data[linearIndexFromCoordinate(x, y-1, z, _sizeX, _sizeY)];
					if (b == b2 &&
						newVerts[numVerts - 6].w == lbf.w &&
						newVerts[numVerts - 5].w == rbf.w &&
						newVerts[numVerts - 4].w == ltf.w &&
						newVerts[numVerts - 1].w == rtf.w)
							potentialMerge = true;
				}
				if (potentialMerge) 
				{
					newVerts[numVerts - 4] = ltf;
					newVerts[numVerts - 3] = ltf;
					newVerts[numVerts - 1] = rtf;
					merged++;
				}
				else 
				{
					newVerts[numVerts++] = lbf;
					newVerts[numVerts++] = rbf;
					newVerts[numVerts++] = ltf;
					newVerts[numVerts++] = ltf;
					newVerts[numVerts++] = rbf;
					newVerts[numVerts++] = rtf;
				}
				vis = true;
			}
		}
	}
	mesh.resize(numVerts);
	mesh.buffer(newVerts, numVerts);
}

uint32_t VoxelData::getPhysicsReduced(Physics& physics, float radius)
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
	Log::Debug("new verts at %p", newVerts);
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

uint32_t VoxelData::getPhysicsCubes(Physics& physics, const float radius)
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

uint32_t VoxelData::getPhysicsHull(Physics& physics, const float radius)
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

uint32_t VoxelData::getPhysicsAABBs(Physics& physics, const glm::vec3& radius)
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
