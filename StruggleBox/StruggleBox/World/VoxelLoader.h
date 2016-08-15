#ifndef VOXEL_LOADER_H
#define VOXEL_LOADER_H

#include <string>
#include <memory>

class VoxelData;

class VoxelLoader
{
public:
	static std::shared_ptr<VoxelData> load(const std::string fileName);
	static void save(
		const std::string fileName,
		std::shared_ptr<VoxelData> data);



};

#endif
