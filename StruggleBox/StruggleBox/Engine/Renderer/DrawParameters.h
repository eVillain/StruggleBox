#pragma once

#include "RendererDefines.h"
#include <tuple>

struct DrawParameters
{
	DrawDataID drawDataID;
	uint8_t textureCount;
	TextureID textureIDs[6];
	ShaderID shaderID;
	BlendMode blendMode;
	DepthMode depthMode;

	bool operator==(const DrawParameters& other) const 
	{
		return drawDataID == other.drawDataID &&
			textureCount == other.textureCount &&
			textureIDs[0] == other.textureIDs[0] &&
			textureIDs[1] == other.textureIDs[1] &&
			textureIDs[2] == other.textureIDs[2] &&
			textureIDs[3] == other.textureIDs[3] &&
			textureIDs[4] == other.textureIDs[4] &&
			textureIDs[5] == other.textureIDs[5] &&
			shaderID == other.shaderID &&
			blendMode == other.blendMode &&
			depthMode == other.depthMode;
	}

	bool operator<(const DrawParameters& other) const {
		return std::tie(
			drawDataID,
			textureCount,
			textureIDs[0],
			textureIDs[1],
			textureIDs[2],
			textureIDs[3],
			textureIDs[4],
			textureIDs[5],
			shaderID, blendMode, depthMode) <
			std::tie(
				other.drawDataID,
				other.textureCount,
				other.textureIDs[0],
				other.textureIDs[1],
				other.textureIDs[2],
				other.textureIDs[3],
				other.textureIDs[4],
				other.textureIDs[5],
				other.shaderID,
				other.blendMode,
				other.depthMode);
	}
};
