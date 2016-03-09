//
//  ParticleSys.cpp
//  Ingenium
//
//  Created by The Drudgerist on 01/03/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "ParticleSys.h"
#include "Dictionary.h"
#include "Texture.h"
#include "TextureManager.h"
#include "Base64.h"
#include "zlib.h"
#include "VertexBuffer.h"
#include "SysCore.h"
#include "GFXHelpers.h"
#include <fstream>              // File input/output

// ugly hack to avoid zlib corruption on win systems
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

ParticleSys::ParticleSys( const std::string filePath, const std::string fileName ) {
    maxParticles = 0;
    position = glm::vec3();
    active = true;
    emissionRate = 1.0f;
    particleCount = 0;
    elapsed = 0.0f;
    emitCounter = 0.0f;
    vBuffer = NULL;
    texture = NULL;
    particles = NULL;
    
    lifeSpan = 0.0f;

    InitFromFile(filePath, fileName);
}

ParticleSys::~ParticleSys() {
    delete [] particles;
    particles = NULL;
    
    if (vBuffer ) {
        delete vBuffer;
        vBuffer = NULL;
    }
    if ( texture ) {
        texture->refCount--;
        TextureManager::Inst()->UnloadTexture(texture);
        texture = NULL;
    }
}
void ParticleSys::InitFromFile( const std::string filePath, const std::string fileName ) {
    if ( vBuffer != NULL ) {
        delete vBuffer;
        vBuffer = NULL;
    }

    std::string dictPath = std::string(filePath).append(fileName);
    
    Dictionary dict;
    if( !dict.loadRootSubDictFromFile( dictPath.c_str() ) ) { return; }

    // Start loading variables
    dimensions = dict.getIntegerForKey("dimensions");
    emitterType = dict.getIntegerForKey("emitterType");
    lighting = dict.getIntegerForKey("lighting");
    
    angle = dict.getFloatForKey("angle");
    angleVar = dict.getFloatForKey("angleVariance");
    blendFuncSrc = dict.getIntegerForKey("blendFuncSource");
    blendFuncDst = dict.getIntegerForKey("blendFuncDestination");
    duration = dict.getFloatForKey("duration");

    finishColor = RGBAColor(dict.getFloatForKey("finishColorRed"),
                            dict.getFloatForKey("finishColorGreen"),
                            dict.getFloatForKey("finishColorBlue"),
                            dict.getFloatForKey("finishColorAlpha"));
    finishColorVar = RGBAColor(dict.getFloatForKey("finishColorVarianceRed"),
                               dict.getFloatForKey("finishColorVarianceGreen"),
                               dict.getFloatForKey("finishColorVarianceBlue"),
                               dict.getFloatForKey("finishColorVarianceAlpha"));
    startColor = RGBAColor(dict.getFloatForKey("startColorRed"),
                           dict.getFloatForKey("startColorGreen"),
                           dict.getFloatForKey("startColorBlue"),
                           dict.getFloatForKey("startColorAlpha"));
    startColorVar = RGBAColor(dict.getFloatForKey("startColorVarianceRed"),
                              dict.getFloatForKey("startColorVarianceGreen"),
                              dict.getFloatForKey("startColorVarianceBlue"),
                              dict.getFloatForKey("startColorVarianceAlpha"));
    
    finishParticleSize = dict.getFloatForKey("finishParticleSize");
    finishParticleSizeVar = dict.getFloatForKey("finishParticleSizeVariance");
    if ( emitterType == ParticleSysGravity ) {
        gravity = glm::vec3(dict.getFloatForKey("gravityx"),
                            dict.getFloatForKey("gravityy"),
                            dict.getFloatForKey("gravityz"));
		//
        // speed
        speed = dict.getFloatForKey("speed");
        speedVar = dict.getFloatForKey("speedVariance");

        // radial acceleration
        radialAccel = dict.getFloatForKey("radialAcceleration");
        radialAccelVar = dict.getFloatForKey("radialAccelVariance");

        // tangential acceleration
        tangAccel = dict.getFloatForKey("tangentialAcceleration");
        tangAccelVar = dict.getFloatForKey("tangentianAccelVariance");
    } else /*if ( emitterType == ParticleSysRadial )*/ {
        maxRadius = dict.getFloatForKey("maxRadius");
        maxRadiusVar = dict.getFloatForKey("maxRadiusVariance");
        minRadius = dict.getFloatForKey("minRadius");
        rotPerSec = dict.getFloatForKey("rotatePerSecond");
        rotPerSecVar = dict.getFloatForKey("rotatePerSecondVariance");
    }
    maxParticles = dict.getIntegerForKey("maxParticles");
    lifeSpan = dict.getFloatForKey("particleLifespan");
    lifeSpanVar = dict.getFloatForKey("particleLifespanVariance");
    rotEnd = dict.getFloatForKey("rotationEnd");
    rotEndVar = dict.getFloatForKey("rotationEndVariance");
    rotStart = dict.getFloatForKey("rotationStart");
    rotStartVar = dict.getFloatForKey("rotationStartVariance");
    sourcePos = glm::vec3(dict.getFloatForKey("sourcePositionx"),
                          dict.getFloatForKey("sourcePositiony"),
                          dict.getFloatForKey("sourcePositionz"));
    sourcePosVar = glm::vec3(dict.getFloatForKey("sourcePositionVariancex"),
                             dict.getFloatForKey("sourcePositionVariancey"),
                             dict.getFloatForKey("sourcePositionVariancez"));
    
    startSize = dict.getFloatForKey("startParticleSize");
    startSizeVar = dict.getFloatForKey("startParticleSizeVariance");

    // Emission rate
    emissionRate = dict.getFloatForKey("emissionRate");
    if ( !emissionRate ) { emissionRate = maxParticles/lifeSpan; }
    
    texFileName = dict.getStringForKey("textureFileName");
    // Check if texture of that name is already loaded
    texture = TextureManager::Inst()->LoadTexture(filePath, texFileName);
    if ( texture == NULL ) {
        printf("[ParticleSys] Loading particle texture %s from data\n", texFileName.c_str() );
        std::string texData = dict.getStringForKey("textureImageData");
        if ( texData.length() != 0 ) {
            // Texture is base64 encoded and zipped image data
            std::string zippedData = base64_decode(texData);
            char* data = new char[128000];
            int zipSize = (int)zippedData.length()+1; // data + NULL delimiter.
            int dataLen = 128000;
            int err;
            z_stream d_stream; // decompression stream
            d_stream.zalloc = (alloc_func)0;
            d_stream.zfree = (free_func)0;
            d_stream.opaque = (voidpf)0;
            d_stream.next_in  = (Bytef*)zippedData.c_str(); // pointer the the compressed data buffer
            d_stream.avail_in = zipSize; // length of the compressed data
            d_stream.next_out = (Bytef*)data; // pointer to the resulting uncompressed data buffer
            d_stream.avail_out = dataLen;
            // Uncompress gzip data ( against all intuition inflate = unzip and deflate = compress in zlib )
            inflateInit2(&d_stream, 16+MAX_WBITS);
            err = inflate (& d_stream, Z_NO_FLUSH);
            if ( err == Z_OK ) {
                printf("[ParticleSys] unzipped image data successfully, size: %lu\n", d_stream.total_out);
            } else if ( err == Z_MEM_ERROR ) {
                printf("[ParticleSys] there was not enough memory\n");
            } else if ( err == Z_BUF_ERROR ) {
                printf("[ParticleSys] there was not enough room in the output buffer\n");
            } else if ( err == Z_DATA_ERROR ) {
                printf("[ParticleSys] the input data was corrupted or incomplete\n");
            } else if ( err == Z_STREAM_END ) {
                printf("[ParticleSys] inflate finished successfully, size: %lu\n", d_stream.total_out);
            }
            err = inflateEnd(&d_stream);
            if ( err != Z_OK ) {
                printf("[ParticleSys] failed to close zip stream\n");
            }
            texture = new Texture("");
            // Try to assess image format from header
            if ( data[0] == -119 && data [1] == 80 ) {
                texture->LoadFromPNGData(data);
            }
			/*else {
                texture->LoadFromTIFFData(data, dataLen);
            }*/
            if ( texture->IsLoaded() ) {
                printf("[ParticleSys] texture loaded\n");
            } else {
                printf("[ParticleSys] texture data error, header:\n"
                       "%i %i %i %i\n %i %i %i %i\n, %i %i %i %i\n %i %i %i %i\n",
                       data[0], data[1], data[2], data[3],
                       data[4], data[5], data[6], data[7],
                       data[8], data[9], data[10], data[11],
                       data[12], data[13], data[14], data[15] );
                bool dumpData = false;
                if ( dumpData ) {
                    // Dump file to disk
                    std::ofstream file2 (texFileName.c_str(), std::ios::out|std::ios::binary|std::ios::ate);
                    if (!file2) {
                    } else if (file2.is_open()) {
                        file2.write((char*)data, dataLen);
                        file2.close();
                    }
                }
            }
			delete[] data;
        }
    }
    if ( texture ) {
        texture->refCount++;
    }
    active = true;
    elapsed = 0;
    if ( !maxParticles ) {
        printf("ParticleSys error: no Max particles");
        maxParticles = 100;
    }
    particles = new Particle[maxParticles];
    vBuffer = new VertexBuffer(-1, RType_SpriteVerts);
    vBuffer->c_verts = new ColorVertexData[maxParticles];
}
void ParticleSys::SaveToFile( const std::string filePath, const std::string fileName ) {
    std::string dictPath = std::string(filePath).append(fileName);
    
    Dictionary dict;
    
    // Start saving variables
    dict.setIntegerForKey("dimensions", dimensions);
    dict.setIntegerForKey("emitterType", emitterType);
    dict.setIntegerForKey("lighting", lighting);
    
    dict.setFloatForKey("angle", angle);
    dict.setFloatForKey("angleVariance", angleVar);
    dict.setIntegerForKey("blendFuncSource", blendFuncSrc);
    dict.setIntegerForKey("blendFuncDestination", blendFuncDst);
    dict.setFloatForKey("duration", duration);
    
    dict.setFloatForKey("finishColorRed", finishColor.r);
    dict.setFloatForKey("finishColorGreen", finishColor.g);
    dict.setFloatForKey("finishColorBlue", finishColor.b);
    dict.setFloatForKey("finishColorAlpha", finishColor.a);
    dict.setFloatForKey("finishColorVarianceRed", finishColorVar.r);
    dict.setFloatForKey("finishColorVarianceGreen", finishColorVar.g);
    dict.setFloatForKey("finishColorVarianceBlue", finishColorVar.b);
    dict.setFloatForKey("finishColorVarianceAlpha", finishColorVar.a);
    dict.setFloatForKey("startColorRed", startColor.r);
    dict.setFloatForKey("startColorGreen", startColor.g);
    dict.setFloatForKey("startColorBlue", startColor.b);
    dict.setFloatForKey("startColorAlpha", startColor.a);
    dict.setFloatForKey("startColorVarianceRed", startColorVar.r);
    dict.setFloatForKey("startColorVarianceGreen", startColorVar.g);
    dict.setFloatForKey("startColorVarianceBlue", startColorVar.b);
    dict.setFloatForKey("startColorVarianceAlpha", startColorVar.a);
    
    dict.setFloatForKey("finishParticleSize", finishParticleSize);
    dict.setFloatForKey("finishParticleSizeVariance", finishParticleSizeVar);
    if ( emitterType == ParticleSysGravity ) {
        dict.setFloatForKey("gravityx", gravity.x);
        dict.setFloatForKey("gravityy", gravity.y);
        dict.setFloatForKey("gravityz", gravity.z);
        dict.setFloatForKey("speed", speed);
        dict.setFloatForKey("speedVariance", speedVar);
        dict.setFloatForKey("radialAcceleration", radialAccel);
        dict.setFloatForKey("radialAccelVariance", radialAccelVar);
        dict.setFloatForKey("tangentialAcceleration", tangAccel);
        dict.setFloatForKey("tangentianAccelVariance", tangAccelVar);
    } else /*if ( emitterType == ParticleSysRadial )*/ {
        dict.setFloatForKey("maxRadius", maxRadius);
        dict.setFloatForKey("maxRadiusVariance", maxRadiusVar);
        dict.setFloatForKey("minRadius", minRadius);
        dict.setFloatForKey("rotatePerSecond", rotPerSec);
        dict.setFloatForKey("rotatePerSecondVariance", rotPerSecVar);
    }
    dict.setIntegerForKey("maxParticles", maxParticles);
    dict.setFloatForKey("particleLifespan", lifeSpan);
    dict.setFloatForKey("particleLifespanVariance", lifeSpanVar);
    dict.setFloatForKey("rotationEnd", rotEnd);
    dict.setFloatForKey("rotationEndVariance", rotEndVar);
    dict.setFloatForKey("rotationStart", rotStart);
    dict.setFloatForKey("rotationStartVariance", rotStartVar);
    dict.setFloatForKey("sourcePositionx", sourcePos.x);
    dict.setFloatForKey("sourcePositiony", sourcePos.y);
    dict.setFloatForKey("sourcePositionz", sourcePos.z);
    dict.setFloatForKey("sourcePositionVariancex", sourcePosVar.x);
    dict.setFloatForKey("sourcePositionVariancey", sourcePosVar.y);
    dict.setFloatForKey("sourcePositionVariancez", sourcePosVar.z);
    
    dict.setFloatForKey("startParticleSize", startSize);
    dict.setFloatForKey("startParticleSizeVariance", startSizeVar);
    
    dict.setFloatForKey("emissionRate", emissionRate);
    
    dict.setStringForKey("textureFileName", texFileName);
    
    dict.saveRootSubDictToFile( dictPath.c_str() );
}


void ParticleSys::Update( const double dt ) {
    if ( vBuffer == NULL ) return;

    if( active && emissionRate ) {
		float rate = 1.0f / emissionRate;
        
		//issue #1201, prevent bursts of particles, due to too high emitCounter
		if (particleCount < maxParticles)
			emitCounter += dt;
        
		while( particleCount < maxParticles && emitCounter > rate ) {
            // TRY TO ADD PARTICLE
            if ( !AddParticle() ) break;
			emitCounter -= rate;
		}
        
		elapsed += dt;
        
		if(duration != -1 && duration < elapsed) {
            // STOP SYSTEM
            StopSystem();
        }
	}
    int particleIdx = 0;
    while ( particleIdx < particleCount ) {
        Particle& p = particles[particleIdx];
        // life
        p.timeToLive -= dt;
        if( p.timeToLive > 0 ) {
            
            // Mode A: gravity, direction, tangential accel & radial accel
            if( emitterType == 0 ) {
                glm::vec3 tmp, radial, tangential;
                
                radial = glm::vec3();
                // radial acceleration
                if(p.pos.x || p.pos.y || p.pos.z)
                    radial = glm::normalize(p.pos);
                
                tangential = radial;
                radial = radial * p.mode.A.radialAccel;
                
                // tangential acceleration
                float newy = tangential.x;
                tangential.x = -tangential.y;
                tangential.y = newy;
                tangential = tangential * p.mode.A.tangentialAccel;
                
                // (gravity + radial + tangential) * dt
                tmp = radial+ tangential + gravity;
                tmp = tmp * (float)dt;
                p.mode.A.dirX = p.mode.A.dirX + tmp.x;
                p.mode.A.dirY = p.mode.A.dirY + tmp.y;
                p.mode.A.dirZ = p.mode.A.dirZ + tmp.z;

                tmp = glm::vec3(p.mode.A.dirX,p.mode.A.dirY,p.mode.A.dirZ) * (float)dt;

                p.pos = p.pos + tmp;
            }
            
            // Mode B: radius movement
            else {
                // Update the angle and radius of the particle.
                p.mode.B.angle += p.mode.B.degreesPerSecond * dt;
                p.mode.B.radius += p.mode.B.deltaRadius * dt;
                
                p.pos.x = - cosf(p.mode.B.angle) * p.mode.B.radius;
                p.pos.y = - sinf(p.mode.B.angle) * p.mode.B.radius;
            }
        
            // color
            p.color.r += (p.deltaColor.r * dt);
            p.color.g += (p.deltaColor.g * dt);
            p.color.b += (p.deltaColor.b * dt);
            p.color.a += (p.deltaColor.a * dt);
            
            // size
            p.size += (p.deltaSize * dt);
            p.size = float_max( 0.0f, p.size );
            
            // angle
            p.rotation += (p.deltaRotation * dt);
            
            //
            // update values in quad
            //
            if ( p.size == 0.0f ) p.size = 0.000000000001f;
            float particleSize = p.size*0.5f;
//            if ( dimensions == ParticleSys3D ) {
//                particleSize /= 128.0f;     // 128 pixels per meter ratio
//            }
            vBuffer->c_verts[particleIdx] = {
                p.pos.x+position.x,p.pos.y+position.y,p.pos.z+position.z,particleSize,
                p.color.r,p.color.g,p.color.b,p.color.a
            };
            // update particle counter
            particleIdx++;
            
        } else {
            // life < 0
            if( particleIdx != particleCount-1 ) {
                // Switch pointer with last in buffer
                particles[particleIdx] = particles[particleCount-1];
            }
            particleCount--;
        }
    }//while
    vBuffer->numVerts = particleCount;
    vBuffer->updated = true;
}
int GetBlendMode( int particleBlend ) {
    if ( particleBlend == 1 ) {
        return GL_ONE;
    } else if ( particleBlend == 770 ) {
        return GL_SRC_ALPHA;
    } else if ( particleBlend == 771 ) {
        return GL_ONE_MINUS_SRC_ALPHA;
    } else if ( particleBlend == 772 ) {
        return GL_DST_ALPHA;
    } else if ( particleBlend == 773 ) {
        return GL_ONE_MINUS_DST_ALPHA;
    } else if ( particleBlend == 775 ) {
        return GL_ONE_MINUS_DST_COLOR;
    } else if ( particleBlend == 776 ) {
        return GL_ONE_MINUS_SRC_COLOR;
    } else {
        return GL_ONE;
    }

}
void ParticleSys::Draw( Renderer *renderer ) {
    if ( vBuffer != NULL ) {
        if ( lighting == ParticleSysLightOn ) { glDisable(GL_BLEND); }
        else { glEnable(GL_BLEND); }
        
        int srcBlend = GetBlendMode(blendFuncSrc);
        int dstBlend = GetBlendMode(blendFuncDst);
        glBlendFunc(srcBlend, dstBlend);
        if ( dimensions == ParticleSys2D ) {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
        } else {
            glEnable(GL_DEPTH_TEST);
            if ( lighting == ParticleSysLightOff ) {
                glDepthMask(GL_FALSE);
            } else {
                glDepthMask(GL_TRUE);
            }
        }

        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
        renderer->RenderVertBuffer(vBuffer, particleCount, 0, texture, dimensions);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}
void ParticleSys::StopSystem() {
    active = false;
    elapsed = 0.0f;
    emitCounter = 0.0f;
}
bool ParticleSys::AddParticle() {
    if ( particleCount == maxParticles ) return false;
    SysCore::RandomSeed((int)SysCore::GetMicroseconds());
    Particle& particle = particles[particleCount];
    
    
    particle.timeToLive = lifeSpan + lifeSpanVar * SysCore::RandomDouble();
	particle.timeToLive = float_max(0.0f, particle.timeToLive);
    
	// position
	particle.pos.x = sourcePos.x + sourcePosVar.x * SysCore::RandomDouble();
	particle.pos.y = sourcePos.y + sourcePosVar.y * SysCore::RandomDouble();
    particle.pos.z = sourcePos.z + sourcePosVar.z * SysCore::RandomDouble();

	// Colorm
	Color start;
	start.r = float_clamp( startColor.r + startColorVar.r * SysCore::RandomDouble(), 0, 1);
	start.g = float_clamp( startColor.g + startColorVar.g * SysCore::RandomDouble(), 0, 1);
	start.b = float_clamp( startColor.b + startColorVar.b * SysCore::RandomDouble(), 0, 1);
	start.a = float_clamp( startColor.a + startColorVar.a * SysCore::RandomDouble(), 0, 1);
    
	Color end;
	end.r = float_clamp( finishColor.r + finishColorVar.r * SysCore::RandomDouble(), 0, 1);
	end.g = float_clamp( finishColor.g + finishColorVar.g * SysCore::RandomDouble(), 0, 1);
	end.b = float_clamp( finishColor.b + finishColorVar.b * SysCore::RandomDouble(), 0, 1);
	end.a = float_clamp( finishColor.a + finishColorVar.a * SysCore::RandomDouble(), 0, 1);
    
	particle.color = start;
	particle.deltaColor.r = (end.r - start.r) / particle.timeToLive;
	particle.deltaColor.g = (end.g - start.g) / particle.timeToLive;
	particle.deltaColor.b = (end.b - start.b) / particle.timeToLive;
	particle.deltaColor.a = (end.a - start.a) / particle.timeToLive;
    
	// size
	float startS = startSize + startSizeVar * SysCore::RandomDouble();
	startS = float_max(0, startS); // No negative value
    
	particle.size = startS;
	if( finishParticleSize == -1 )
		particle.deltaSize = 0;
	else {
		float endS = finishParticleSize + finishParticleSizeVar * SysCore::RandomDouble();
		endS = float_max(0, endS);	// No negative values
		particle.deltaSize = (endS - startS) / particle.timeToLive;
	}
    
	// rotation
	float startA = rotStart + rotStartVar * SysCore::RandomDouble();
	float endA = rotEnd + rotEndVar * SysCore::RandomDouble();
	particle.rotation = startA;
	particle.deltaRotation = (endA - startA) / particle.timeToLive;
    
	// position
//	if( particlePositionType == -1 )
//		particle->startPos = [self convertToWorldSpace:CGPointZero];
//	else
//        if( _particlePositionType == CCParticleSystemPositionTypeRelative )
//		particle->startPos = position;
    
	// direction
	float a = toRads( angle + angleVar * SysCore::RandomDouble() );
    
	// Mode Gravity: A
	if( emitterType == 0 ) {
        
        glm::vec3 v = glm::vec3(cosf( a ), sinf( a ), 0.0f);
		float s = speed + speedVar * SysCore::RandomDouble();
        
		// direction
		particle.mode.A.dirX =  v.x * s;
        particle.mode.A.dirY =  v.y * s;
		particle.mode.A.dirZ =  v.z * s;

		// radial accel
		particle.mode.A.radialAccel = radialAccel + radialAccelVar * SysCore::RandomDouble();
        
		// tangential accel
		particle.mode.A.tangentialAccel = tangAccel + tangAccelVar * SysCore::RandomDouble();
	}
	// Mode Radius: B
	else {
		// Set the default diameter of the particle from the source position
		float startRadius = maxRadius + maxRadiusVar * SysCore::RandomDouble();
		float endRadius = minRadius + minRadiusVar * SysCore::RandomDouble();
        
		particle.mode.B.radius = startRadius;
        
		if( endRadius == ParticleSystemStartRadiusEqualToEndRadius )
			particle.mode.B.deltaRadius = 0;
		else
			particle.mode.B.deltaRadius = (endRadius - startRadius) / particle.timeToLive;
        
		particle.mode.B.angle = a;
		particle.mode.B.degreesPerSecond = toRads(rotPerSec + rotPerSecVar * SysCore::RandomDouble());
	}
    
    particleCount++;
    return true;
}

