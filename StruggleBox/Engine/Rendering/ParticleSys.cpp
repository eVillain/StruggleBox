#include "ParticleSys.h"

#include "Dictionary.h"
#include "CoreIncludes.h"
#include "GFXHelpers.h"
#include "Random.h"
#include "Log.h"


ParticleSystem::ParticleSystem(const ParticleSystemConfig& config, Particle* particleData)
    : m_config(config)
	, m_particleCount(0)
	, m_active(false)
	, m_elapsed(0.f)
	, m_emitCounter(0.f)
	, m_particles(particleData)
	, m_dirty(false)
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Update(const double dt)
{
	if (m_active && m_config.emissionRate > 0.f)
	{
		const float rate = 1.0f / m_config.emissionRate;
		// prevent bursts of particles due to too high emitCounter value
		if (m_particleCount < m_config.maxParticles)
		{
			m_emitCounter += dt;
		}

		while (m_particleCount < m_config.maxParticles && m_emitCounter > rate)
		{
			if (!AddParticle()) break;
			m_emitCounter -= rate;
		}
	}
	else if (!m_active && m_particleCount == 0)
	{
		return;
	}

	m_elapsed += dt;

	if (m_config.duration != -1 && m_config.duration < m_elapsed)
	{
		StopSystem();
	}

    size_t particleIdx = 0;
    while (particleIdx < m_particleCount)
	{
        Particle& p = m_particles[particleIdx];
        // life
        p.timeToLive -= dt;
        if (p.timeToLive > 0.f) 
		{
            // Mode A: gravity, direction, tangential accel & radial accel
            if (m_config.emitterType == ParticleSysMode::ParticleSysGravity)
			{
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
                tmp = radial+ tangential + m_config.gravity;
                tmp = tmp * (float)dt;
                p.mode.A.dirX = p.mode.A.dirX + tmp.x;
                p.mode.A.dirY = p.mode.A.dirY + tmp.y;
                p.mode.A.dirZ = p.mode.A.dirZ + tmp.z;

                tmp = glm::vec3(p.mode.A.dirX,p.mode.A.dirY,p.mode.A.dirZ) * (float)dt;

                p.pos = p.pos + tmp;
            }
            
            // Mode B: radius movement
            else 
			{
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
            p.size = std::max(0.0f, p.size);
            
            // angle
            p.rotation += (p.deltaRotation * dt);
            
            // update particle counter
            particleIdx++;
			m_dirty = true;
        }
		else // life < 0
		{
			if (particleIdx != m_particleCount-1)
			{
                // Swap last particle in buffer into this slot
				m_particles[particleIdx] = m_particles[m_particleCount - 1];
            }
			else 
			{
				return;
			}
			m_particleCount--;
        }
    } // while
}

uint32_t GetBlendMode(int particleBlend)
{
	if (particleBlend == 1) {
		return GL_ONE;
	}
	else if (particleBlend == 770) {
		return GL_SRC_ALPHA;
	}
	else if (particleBlend == 771) {
		return GL_ONE_MINUS_SRC_ALPHA;
	}
	else if (particleBlend == 772) {
		return GL_DST_ALPHA;
	}
	else if (particleBlend == 773) {
		return GL_ONE_MINUS_DST_ALPHA;
	}
	else if (particleBlend == 775) {
		return GL_ONE_MINUS_DST_COLOR;
	}
	else if (particleBlend == 776) {
		return GL_ONE_MINUS_SRC_COLOR;
	}
	else {
		return GL_ONE;
	}
}

BlendMode ParticleSystem::getBlendMode() const
{
	return {
		GetBlendMode(m_config.blendFuncSrc),
		GetBlendMode(m_config.blendFuncDst),
		m_config.lighting == ParticleSysLighting::ParticleSysLightOff
	};
}

DepthMode ParticleSystem::getDepthMode() const
{
	return {
		true,
		m_config.lighting == ParticleSysLighting::ParticleSysLightOn
	};
}

void ParticleSystem::StopSystem()
{
	m_active = false;
	m_emitCounter = 0.0f;
}

bool ParticleSystem::AddParticle()
{
	if (m_particleCount == m_config.maxParticles)
	{
		return false;
	}

	Particle& particle = m_particles[m_particleCount];

    particle.timeToLive = m_config.lifeSpan + m_config.lifeSpanVar * Random::RandomDouble();
	particle.timeToLive = std::max(0.0f, particle.timeToLive);
    
	// position
	particle.pos.x = m_config.sourcePos.x + m_config.sourcePosVar.x * Random::RandomDouble();
	particle.pos.y = m_config.sourcePos.y + m_config.sourcePosVar.y * Random::RandomDouble();
    particle.pos.z = m_config.sourcePos.z + m_config.sourcePosVar.z * Random::RandomDouble();

	// Colorm
	Color start;
	start.r = float_clamp(m_config.startColor.r + m_config.startColorVar.r * Random::RandomDouble(), 0, 1);
	start.g = float_clamp(m_config.startColor.g + m_config.startColorVar.g * Random::RandomDouble(), 0, 1);
	start.b = float_clamp(m_config.startColor.b + m_config.startColorVar.b * Random::RandomDouble(), 0, 1);
	start.a = float_clamp(m_config.startColor.a + m_config.startColorVar.a * Random::RandomDouble(), 0, 1);
    
	Color end;
	end.r = float_clamp(m_config.finishColor.r + m_config.finishColorVar.r * Random::RandomDouble(), 0, 1);
	end.g = float_clamp(m_config.finishColor.g + m_config.finishColorVar.g * Random::RandomDouble(), 0, 1);
	end.b = float_clamp(m_config.finishColor.b + m_config.finishColorVar.b * Random::RandomDouble(), 0, 1);
	end.a = float_clamp(m_config.finishColor.a + m_config.finishColorVar.a * Random::RandomDouble(), 0, 1);
    
	particle.color = start;
	particle.deltaColor.r = (end.r - start.r) / particle.timeToLive;
	particle.deltaColor.g = (end.g - start.g) / particle.timeToLive;
	particle.deltaColor.b = (end.b - start.b) / particle.timeToLive;
	particle.deltaColor.a = (end.a - start.a) / particle.timeToLive;
    
	// size
	float startS = m_config.startSize + m_config.startSizeVar * Random::RandomDouble();
	startS = float_max(0, startS); // No negative value
    
	particle.size = startS;
	if (m_config.finishSize == -1)
	{
		particle.deltaSize = 0;
	}
	else 
	{
		float endS = m_config.finishSize + m_config.finishSizeVar * Random::RandomDouble();
		endS = float_max(0, endS);	// No negative values
		particle.deltaSize = (endS - startS) / particle.timeToLive;
	}
    
	// rotation
	const float startA = m_config.rotStart + m_config.rotStartVar * Random::RandomDouble();
	const float endA = m_config.rotEnd + m_config.rotEndVar * Random::RandomDouble();
	particle.rotation = startA;
	particle.deltaRotation = (endA - startA) / particle.timeToLive;
    
	// direction
	const float a = toRads(m_config.angle + m_config.angleVar * Random::RandomDouble());
    
	// Mode Gravity: A
	if(m_config.emitterType == ParticleSysMode::ParticleSysGravity)
	{
        
        glm::vec3 v = glm::vec3(cosf( a ), sinf( a ), 0.0f);
		float s = m_config.speed + m_config.speedVar * Random::RandomDouble();
        
		// direction
		particle.mode.A.dirX =  v.x * s;
        particle.mode.A.dirY =  v.y * s;
		particle.mode.A.dirZ =  v.z * s;

		// radial accel
		particle.mode.A.radialAccel = m_config.radialAccel + m_config.radialAccelVar * Random::RandomDouble();
        
		// tangential accel
		particle.mode.A.tangentialAccel = m_config.tangAccel + m_config.tangAccelVar * Random::RandomDouble();
	}
	// Mode Radius: B
	else 
	{
		// Set the default diameter of the particle from the source position
		const float startRadius = m_config.maxRadius + m_config.maxRadiusVar * Random::RandomDouble();
		const float endRadius = m_config.minRadius + m_config.minRadiusVar * Random::RandomDouble();
        
		particle.mode.B.radius = startRadius;
        
		if( endRadius == -1.f )
			particle.mode.B.deltaRadius = 0;
		else
			particle.mode.B.deltaRadius = (endRadius - startRadius) / particle.timeToLive;
        
		particle.mode.B.angle = a;
		particle.mode.B.degreesPerSecond = toRads(m_config.rotPerSec + m_config.rotPerSecVar * Random::RandomDouble());
	}
    
	m_particleCount++;
    return true;
}

