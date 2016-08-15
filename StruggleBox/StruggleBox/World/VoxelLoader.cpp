#include "VoxelLoader.h"
#include "VoxelData.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "Serialise.h"
#include "Log.h"

#define OLD_CUBESET_HEADER "BWOCB"  // Must be 5 characters
#define OLD_CUBESET_VERSION "1.0"   // Must be 3 characters

bool checkHeaderTag(const unsigned char* buffer)
{
	return strncmp((const char*)buffer, VOXELDATA_HEADER, 5) == 0;
}
bool checkHeaderVersion(const unsigned char* buffer)
{
	return strncmp((const char*)buffer, VOXELDATA_VERSION, 3) == 0;
}
bool checkHeaderTagOld(const unsigned char* buffer)
{
	return strncmp((const char*)buffer, OLD_CUBESET_HEADER, 5) == 0;
}
bool checkHeaderVersionOld(const unsigned char* buffer)
{
	return strncmp((const char*)buffer, OLD_CUBESET_VERSION, 3) == 0;
}
static inline glm::ivec3 getOldIndexPos(int index, const int width_bits, const int height_bits)
{
    int x = index >> (width_bits + height_bits);
    int y = (index >> width_bits) & ((1 << height_bits) - 1);
    int z = index & ((1 << width_bits) - 1);
    return glm::ivec3(x,y,z);
};

std::shared_ptr<VoxelData> VoxelLoader::load(const std::string fileName)
{
	const std::string absolutePath = PathUtil::getObjectPath(fileName);

	std::shared_ptr<VoxelData> data;
	std::ifstream::int_type fileSize;
	unsigned char * buffer = NULL;
	std::ifstream file(absolutePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

	if (file && file.is_open())
	{
		fileSize = (int)file.tellg();
		buffer = new unsigned char[fileSize];
		file.seekg(0, std::ios::beg);
		file.read((char*)buffer, fileSize);
		file.close();

		Log::Debug("[VoxelLoader] Loaded voxel data file %s", fileName.c_str());

		unsigned int readBytes = 0;
		// Read header and version number
		if (checkHeaderTag(buffer) &&
			checkHeaderVersion(buffer + 5))
		{
			readBytes = 8;
			// Load width and height of object
			const int sizeX = Serialise::deserialiseInt(buffer + readBytes);
			readBytes += SERIALISED_INT_SIZE;
			const int sizeY = Serialise::deserialiseInt(buffer + readBytes);
			readBytes += SERIALISED_INT_SIZE;
			const int sizeZ = Serialise::deserialiseInt(buffer + readBytes);
			readBytes += SERIALISED_INT_SIZE;

			// Prepare block storage
			data = std::make_shared<VoxelData>(sizeX, sizeY, sizeZ);
			// Load number of cubes
			unsigned int numCubes = sizeX*sizeY*sizeZ;

			if (numCubes <= fileSize - readBytes)
			{
				data->setData(buffer + readBytes, numCubes);
			}
			else
			{
				Log::Error("[VoxelLoader] Bad voxel data, cubes: %i, file: %i, header: %i",
					numCubes, fileSize, readBytes);
			}
		}
		else if (checkHeaderTagOld(buffer) &&
			checkHeaderVersionOld(buffer + 5))
		{
			// Old object data type, try to deserialize

			readBytes = 8;
			// Load width and height of object
			const int width_bits = Serialise::deserialiseInt(buffer + readBytes);
			readBytes += SERIALISED_INT_SIZE;
			const int height_bits = Serialise::deserialiseInt(buffer + readBytes);
			readBytes += SERIALISED_INT_SIZE;
			// Load number of cubes
			unsigned int numCubes = Serialise::deserialiseInt(buffer + readBytes);
			readBytes += SERIALISED_INT_SIZE;
			// Prepare block storage
			data = std::make_shared<VoxelData>(pow(2.0,width_bits), pow(2.0, height_bits), pow(2.0, width_bits));

			// Load block data
			for (unsigned int i = 0; i<numCubes; i++) {
				const glm::ivec3 oldCoord = getOldIndexPos(i, width_bits, height_bits);
				unsigned int block = Serialise::deserialiseInt(buffer + readBytes);
				(*data)(oldCoord.x, oldCoord.y, oldCoord.z) = block;
				readBytes += SERIALISED_INT_SIZE;
				// Skip unused color data
				readBytes += SERIALISED_FLOAT_SIZE;
				readBytes += SERIALISED_FLOAT_SIZE;
				readBytes += SERIALISED_FLOAT_SIZE;
				readBytes += SERIALISED_FLOAT_SIZE;
			}
		}
		else
		{
			Log::Error("[VoxelLoader] Object header fail!");
		}
		delete[] buffer;
	}
	else
	{
		Log::Error("[VoxelLoader] Can't load file:%s", absolutePath.c_str());
	}
	return data;
}

void VoxelLoader::save(
	const std::string fileName,
	std::shared_ptr<VoxelData> data)
{
	const std::string absolutePath = PathUtil::getObjectPath(fileName);

	const int headerSize = 8 + (SERIALISED_INT_SIZE * 3);
	const int requiredSize = data->getSizeX()*data->getSizeY()*data->getSizeZ() + headerSize;
	int dataSize = 0;
	unsigned char* buffer = new unsigned char[requiredSize];

	// Save 5 byte header and 3 byte version
	memcpy(buffer, VOXELDATA_HEADER, 5);
	memcpy(buffer + 5, VOXELDATA_VERSION, 3);
	dataSize += 8;

	// Save width and height of object
	dataSize += Serialise::serialise((unsigned int)data->getSizeX(), buffer + dataSize);
	dataSize += Serialise::serialise((unsigned int)data->getSizeY(), buffer + dataSize);
	dataSize += Serialise::serialise((unsigned int)data->getSizeZ(), buffer + dataSize);

	// Save cube data
	const int voxelDataSize = data->getSizeX()*data->getSizeY()*data->getSizeZ();
	memcpy(buffer + dataSize, data->getData(), voxelDataSize);
	dataSize += voxelDataSize;
	Log::Debug("[VoxelLoader] Saved data %i, predicted: %i", dataSize, requiredSize);

	std::ofstream file(absolutePath.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
	if (!file || (file.rdstate() & std::ifstream::failbit ) != 0 )
	{
		Log::Error("[VoxelLoader] Error saving to file %s", absolutePath.c_str());
	}
	else if (file.is_open()) {
		file.write((char*)buffer, dataSize);
		file.close();
		Log::Debug("[VoxelLoader] voxel content saved, size: %i", dataSize);
	}
}
