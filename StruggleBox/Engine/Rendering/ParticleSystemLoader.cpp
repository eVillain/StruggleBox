#include "ParticleSystemLoader.h"

#include "ParticleSys.h"
#include "Dictionary.h"
#include "Texture2D.h"
#include "Base64.h"
#include "Log.h"
#include <fstream>              // File input/output

ParticleSystemConfig ParticleSystemLoader::load(const std::string& fileName)
{
    ParticleSystemConfig config;

    Dictionary dict;
    if(!dict.loadRootSubDictFromFile(fileName.c_str()))
    {
        Log::Error("ParticleSystemLoader::load failed to load %s", fileName);
        config.maxParticles = 0;
        return config;
    }
    // Start loading variables
    config.dimensions = (ParticleSysDimensions)dict.getIntegerForKey("dimensions");
    config.emitterType = (ParticleSysMode)dict.getIntegerForKey("emitterType");
    config.lighting = (ParticleSysLighting)dict.getIntegerForKey("lighting");

    config.angle = dict.getFloatForKey("angle");
    config.angleVar = dict.getFloatForKey("angleVariance");
    config.blendFuncSrc = dict.getIntegerForKey("blendFuncSource");
    config.blendFuncDst = dict.getIntegerForKey("blendFuncDestination");
    config.duration = dict.getFloatForKey("duration");

    config.finishColor = RGBAColor(
        dict.getFloatForKey("finishColorRed"),
        dict.getFloatForKey("finishColorGreen"),
        dict.getFloatForKey("finishColorBlue"),
        dict.getFloatForKey("finishColorAlpha")
    );
    config.finishColorVar = RGBAColor(
        dict.getFloatForKey("finishColorVarianceRed"),
        dict.getFloatForKey("finishColorVarianceGreen"),
        dict.getFloatForKey("finishColorVarianceBlue"),
        dict.getFloatForKey("finishColorVarianceAlpha")
    );
    config.startColor = RGBAColor(
        dict.getFloatForKey("startColorRed"),
        dict.getFloatForKey("startColorGreen"),
        dict.getFloatForKey("startColorBlue"),
        dict.getFloatForKey("startColorAlpha"))
        ;
    config.startColorVar = RGBAColor(
        dict.getFloatForKey("startColorVarianceRed"),
        dict.getFloatForKey("startColorVarianceGreen"),
        dict.getFloatForKey("startColorVarianceBlue"),
        dict.getFloatForKey("startColorVarianceAlpha")
    );

    config.startSize = dict.getFloatForKey("startParticleSize");
    config.startSizeVar = dict.getFloatForKey("startParticleSizeVariance");
    config.finishSize = dict.getFloatForKey("finishParticleSize");
    config.finishSizeVar = dict.getFloatForKey("finishParticleSizeVariance");

    config.maxParticles = dict.getIntegerForKey("maxParticles");
    config.lifeSpan = dict.getFloatForKey("particleLifespan");
    config.lifeSpanVar = dict.getFloatForKey("particleLifespanVariance");
    config.rotEnd = dict.getFloatForKey("rotationEnd");
    config.rotEndVar = dict.getFloatForKey("rotationEndVariance");
    config.rotStart = dict.getFloatForKey("rotationStart");
    config.rotStartVar = dict.getFloatForKey("rotationStartVariance");
    config.sourcePos = glm::vec3(
        dict.getFloatForKey("sourcePositionx"),
        dict.getFloatForKey("sourcePositiony"),
        dict.getFloatForKey("sourcePositionz")
    );
    config.sourcePosVar = glm::vec3(
        dict.getFloatForKey("sourcePositionVariancex"),
        dict.getFloatForKey("sourcePositionVariancey"),
        dict.getFloatForKey("sourcePositionVariancez")
    );

    // Emission rate
    config.emissionRate = dict.getFloatForKey("emissionRate");
    if (config.emissionRate == 0.f)
    {
        config.emissionRate = config.maxParticles / config.lifeSpan;
    }

    if (config.emitterType == ParticleSysMode::ParticleSysGravity)
    {
        config.gravity = glm::vec3(
            dict.getFloatForKey("gravityx"),
            dict.getFloatForKey("gravityy"),
            dict.getFloatForKey("gravityz")
        );
        config.speed = dict.getFloatForKey("speed");
        config.speedVar = dict.getFloatForKey("speedVariance");
        config.radialAccel = dict.getFloatForKey("radialAcceleration");
        config.radialAccelVar = dict.getFloatForKey("radialAccelVariance");
        config.tangAccel = dict.getFloatForKey("tangentialAcceleration");
        config.tangAccelVar = dict.getFloatForKey("tangentianAccelVariance");
    }
    else /*if ( emitterType == ParticleSysRadial )*/
    {
        config.maxRadius = dict.getFloatForKey("maxRadius");
        config.maxRadiusVar = dict.getFloatForKey("maxRadiusVariance");
        config.minRadius = dict.getFloatForKey("minRadius");
        config.minRadiusVar = dict.getFloatForKey("minRadiusVariance");
        config.rotPerSec = dict.getFloatForKey("rotatePerSecond");
        config.rotPerSecVar = dict.getFloatForKey("rotatePerSecondVariance");
    }
    
    const size_t fileNPos = fileName.find_last_of("/");
    const std::string filePath = fileName.substr(0, fileNPos + 1);
    config.texFileName = filePath + dict.getStringForKey("textureFileName");


    return config;

    // Check if texture of that name is already loaded
    //const TextureID textureID = _renderer.getTextureID(filePath + texFileName);
   // if ( textureID == 0 )
   // {
   //     printf("[ParticleSys] Loading particle texture %s from data\n", texFileName.c_str() );
   //     std::string texData = dict.getStringForKey("textureImageData");
   //     if ( texData.length() != 0 ) {
   //         // Texture is base64 encoded and zipped image data
   //         std::string zippedData = base64_decode(texData);
   //         char* data = new char[128000];
   //         int zipSize = (int)zippedData.length()+1; // data + NULL delimiter.
   //         int dataLen = 128000;
   //         int err;
   //         z_stream d_stream; // decompression stream
   //         d_stream.zalloc = (alloc_func)0;
   //         d_stream.zfree = (free_func)0;
   //         d_stream.opaque = (voidpf)0;
   //         d_stream.next_in  = (Bytef*)zippedData.c_str(); // pointer the the compressed data buffer
   //         d_stream.avail_in = zipSize; // length of the compressed data
   //         d_stream.next_out = (Bytef*)data; // pointer to the resulting uncompressed data buffer
   //         d_stream.avail_out = dataLen;
   //         // Uncompress gzip data ( against all intuition inflate = unzip and deflate = compress in zlib )
   //         inflateInit2(&d_stream, 16+MAX_WBITS);
   //         err = inflate (& d_stream, Z_NO_FLUSH);
   //         if ( err == Z_OK ) {
   //             printf("[ParticleSys] unzipped image data successfully, size: %lu\n", d_stream.total_out);
   //         } else if ( err == Z_MEM_ERROR ) {
   //             printf("[ParticleSys] there was not enough memory\n");
   //         } else if ( err == Z_BUF_ERROR ) {
   //             printf("[ParticleSys] there was not enough room in the output buffer\n");
   //         } else if ( err == Z_DATA_ERROR ) {
   //             printf("[ParticleSys] the input data was corrupted or incomplete\n");
   //         } else if ( err == Z_STREAM_END ) {
   //             printf("[ParticleSys] inflate finished successfully, size: %lu\n", d_stream.total_out);
   //         }
   //         err = inflateEnd(&d_stream);
   //         if ( err != Z_OK ) {
   //             printf("[ParticleSys] failed to close zip stream\n");
   //         }
   //         texture2D = new Texture("");
   //         // Try to assess image format from header
   //         if ( data[0] == -119 && data [1] == 80 ) {
   //             texture->LoadFromPNGData(data);
   //         }
			///*else {
   //             texture->LoadFromTIFFData(data, dataLen);
   //         }*/
   //         if ( texture->IsLoaded() ) {
   //             printf("[ParticleSys] texture loaded\n");
   //         } else {
   //             printf("[ParticleSys] texture data error, header:\n"
   //                    "%i %i %i %i\n %i %i %i %i\n, %i %i %i %i\n %i %i %i %i\n",
   //                    data[0], data[1], data[2], data[3],
   //                    data[4], data[5], data[6], data[7],
   //                    data[8], data[9], data[10], data[11],
   //                    data[12], data[13], data[14], data[15] );
   //             bool dumpData = false;
   //             if ( dumpData ) {
   //                 // Dump file to disk
   //                 std::ofstream file2 (texFileName.c_str(), std::ios::out|std::ios::binary|std::ios::ate);
   //                 if (!file2) {
   //                 } else if (file2.is_open()) {
   //                     file2.write((char*)data, dataLen);
   //                     file2.close();
   //                 }
   //             }
   //         }
			//delete[] data;
        //}
    //}
}

void ParticleSystemLoader::save(const ParticleSystemConfig& config, const std::string& fileName)
{
    Dictionary dict;
    
    // Start saving variables
    dict.setIntegerForKey("dimensions", (int)config.dimensions);
    dict.setIntegerForKey("emitterType", (int)config.emitterType);
    dict.setIntegerForKey("lighting", (int)config.lighting);
    
    dict.setFloatForKey("angle", config.angle);
    dict.setFloatForKey("angleVariance", config.angleVar);
    dict.setIntegerForKey("blendFuncSource", config.blendFuncSrc);
    dict.setIntegerForKey("blendFuncDestination", config.blendFuncDst);
    dict.setFloatForKey("duration", config.duration);
    
    dict.setFloatForKey("finishColorRed", config.finishColor.r);
    dict.setFloatForKey("finishColorGreen", config.finishColor.g);
    dict.setFloatForKey("finishColorBlue", config.finishColor.b);
    dict.setFloatForKey("finishColorAlpha", config.finishColor.a);
    dict.setFloatForKey("finishColorVarianceRed", config.finishColorVar.r);
    dict.setFloatForKey("finishColorVarianceGreen", config.finishColorVar.g);
    dict.setFloatForKey("finishColorVarianceBlue", config.finishColorVar.b);
    dict.setFloatForKey("finishColorVarianceAlpha", config.finishColorVar.a);
    dict.setFloatForKey("startColorRed", config.startColor.r);
    dict.setFloatForKey("startColorGreen", config.startColor.g);
    dict.setFloatForKey("startColorBlue", config.startColor.b);
    dict.setFloatForKey("startColorAlpha", config.startColor.a);
    dict.setFloatForKey("startColorVarianceRed", config.startColorVar.r);
    dict.setFloatForKey("startColorVarianceGreen", config.startColorVar.g);
    dict.setFloatForKey("startColorVarianceBlue", config.startColorVar.b);
    dict.setFloatForKey("startColorVarianceAlpha", config.startColorVar.a);

    dict.setFloatForKey("startParticleSize", config.startSize);
    dict.setFloatForKey("startParticleSizeVariance", config.startSizeVar);
    dict.setFloatForKey("finishParticleSize", config.finishSize);
    dict.setFloatForKey("finishParticleSizeVariance", config.finishSizeVar);
    if (config.emitterType == ParticleSysMode::ParticleSysGravity)
    {
        dict.setFloatForKey("gravityx", config.gravity.x);
        dict.setFloatForKey("gravityy", config.gravity.y);
        dict.setFloatForKey("gravityz", config.gravity.z);
        dict.setFloatForKey("speed", config.speed);
        dict.setFloatForKey("speedVariance", config.speedVar);
        dict.setFloatForKey("radialAcceleration", config.radialAccel);
        dict.setFloatForKey("radialAccelVariance", config.radialAccelVar);
        dict.setFloatForKey("tangentialAcceleration", config.tangAccel);
        dict.setFloatForKey("tangentianAccelVariance", config.tangAccelVar);
    } 
    else /*if ( emitterType == ParticleSysRadial )*/ 
    {
        dict.setFloatForKey("maxRadius", config.maxRadius);
        dict.setFloatForKey("maxRadiusVariance", config.maxRadiusVar);
        dict.setFloatForKey("minRadius", config.minRadius);
        dict.setFloatForKey("rotatePerSecond", config.rotPerSec);
        dict.setFloatForKey("rotatePerSecondVariance", config.rotPerSecVar);
    }
    dict.setIntegerForKey("maxParticles", config.maxParticles);
    dict.setFloatForKey("particleLifespan", config.lifeSpan);
    dict.setFloatForKey("particleLifespanVariance", config.lifeSpanVar);
    dict.setFloatForKey("rotationEnd", config.rotEnd);
    dict.setFloatForKey("rotationEndVariance", config.rotEndVar);
    dict.setFloatForKey("rotationStart", config.rotStart);
    dict.setFloatForKey("rotationStartVariance", config.rotStartVar);
    dict.setFloatForKey("sourcePositionx", config.sourcePos.x);
    dict.setFloatForKey("sourcePositiony", config.sourcePos.y);
    dict.setFloatForKey("sourcePositionz", config.sourcePos.z);
    dict.setFloatForKey("sourcePositionVariancex", config.sourcePosVar.x);
    dict.setFloatForKey("sourcePositionVariancey", config.sourcePosVar.y);
    dict.setFloatForKey("sourcePositionVariancez", config.sourcePosVar.z);

    
    dict.setFloatForKey("emissionRate", config.emissionRate);
    
    dict.setStringForKey("textureFileName", config.texFileName);
    
    if (!dict.saveRootSubDictToFile(fileName.c_str()))
    {
        Log::Error("ParticleSystemLoader::save failed to save %s", fileName);
    }
}
