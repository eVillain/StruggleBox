#ifndef VOXEL_LOADER_H
#define VOXEL_LOADER_H

#include <string>
#include <memory>

class Allocator;
class VoxelData;

class VoxelLoader
{
public:
	static VoxelData* load(const std::string& fileName, Allocator& allocator);
	static void save(const std::string& fileName, const VoxelData* data, Allocator& allocator);



};

#endif
