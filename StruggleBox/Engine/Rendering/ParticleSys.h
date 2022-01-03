#pragma once

#include "GFXDefines.h"
#include "Texture2D.h"
#include "Renderer.h"
#include "VertexData.h"
#include "Dictionary.h"
#include <vector>
#include <queue>
#include <memory>

/// Contains the values of each individual particle.
typedef struct sParticle {
    glm::vec3   pos;
    glm::vec3   startPos;
	Color       color;
	Color       deltaColor;
	float		size;
	float		deltaSize;
	float		rotation;
	float		deltaRotation;
	float		timeToLive;
    union {
		struct { // Mode A
            float       dirX, dirY, dirZ;
			float		radialAccel;
			float		tangentialAccel;
		} A;
		struct { // Mode B
			float		angle;
			float		degreesPerSecond;
			float		radius;
			float		deltaRadius;
		} B;
	} mode;
} Particle;

enum class ParticleSysMode {          // System mode (Gravity or Radial)
    ParticleSysGravity = 0,
    ParticleSysRadial = 1
};

enum class ParticleSysDimensions {    // System dimensions (2D or 3D)
    ParticleSys2D = 0,
    ParticleSys3D = 1
};
enum class ParticleSysLighting {      // System lighting (self lit or requires lighting pass)
    ParticleSysLightOff = 0,
    ParticleSysLightOn = 1
};

struct ParticleSystemConfig
{
    int maxParticles;
    ParticleSysDimensions dimensions;     // 0=2D or 1=3D
    ParticleSysLighting lighting;       // 0 = self-lit, 1 = needs lighting
    ParticleSysMode emitterType;          // Gravity or Radial
    float emissionRate; // Rate of particle emission
    float elapsed;      // Amount of time system has run
    float emitCounter;  // Time remaining for particle emission
    float angle, angleVar;  // Particle spawn angle and variance
    int blendFuncSrc, blendFuncDst;
    float duration;
    Color startColor, startColorVar;
    Color finishColor, finishColorVar;
    float startSize, startSizeVar;  // Start size and variance
    float finishSize, finishSizeVar;
    float lifeSpan, lifeSpanVar;
    float rotEnd, rotEndVar;
    float rotStart, rotStartVar;
    glm::vec3 sourcePos, sourcePosVar;

    float maxRadius, maxRadiusVar;
    float minRadius, minRadiusVar;
    float rotPerSec, rotPerSecVar;

    glm::vec3 gravity;
    float speed, speedVar;
    float radialAccel, radialAccelVar;
    float tangAccel, tangAccelVar;  // Tangential acceleration and variance

    std::string texFileName;    // Texture filename
};

class Allocator;

///  Defines a particle system and particles contained within
///  Heavily inspired by Cocos2D and Particle Designer
class ParticleSystem
{
public:
    ParticleSystem(const ParticleSystemConfig& config, Particle* particleData);
    ~ParticleSystem();

    void Update(const double deltaTime);

    void StopSystem();

    BlendMode getBlendMode() const;
    DepthMode getDepthMode() const;

    Particle* getParticles() { return m_particles; }

    bool getIsDirty() const { return m_dirty; }
    void setIsDirty(bool dirty) { m_dirty = dirty; }

    size_t getParticleCount() const { return m_particleCount; }
    const glm::vec3& getPosition() const { return m_config.sourcePos; }
    void setPosition(const glm::vec3& position) { m_config.sourcePos = position; }
    const glm::vec3& getSourcePosVar() const { return m_config.sourcePosVar; }

    float getDuration() const { return m_config.duration; }
    void setDuration(float duration) { m_config.duration = duration; }
    ParticleSysLighting getLighting() const { return m_config.lighting; }
    void setSpeed(float speed) { m_config.speed = speed; }
    void setActive(bool active) { m_active = active; }

private:
    ParticleSystemConfig m_config;

    size_t m_particleCount;  // Current number of particles

    bool m_active;        // Is particle system active
    float m_elapsed;      // Amount of time system has run
    float m_emitCounter;  // Time remaining for particle emission

	Particle* m_particles;        // Array of particles
	bool m_dirty;

	bool AddParticle();
};
