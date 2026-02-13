#include "ExplosiveComponent.h"
#include "EntityManager.h"
#include "ParticleComponent.h"
#include "PhysicsComponent.h"
#include "Physics.h"

ExplosiveComponent::ExplosiveComponent(
	const int ownerID,
	EntityManager& manager,
	Particles& particles) :
EntityComponent(ownerID, "Explosive"),
_manager(manager),
_particles(particles)
{
    _timer = 0.0;
}

ExplosiveComponent::~ExplosiveComponent()
{ }

void ExplosiveComponent::update(const double delta)
{
    if ( _timer != 0.0 )
	{
        _timer -= delta;
        if ( _timer <= 0.0 )
		{ // EXPLODED
            Entity* _owner = _manager.getEntity(_ownerID);
            glm::vec3 pos = _owner->GetAttributeDataPtr<glm::vec3>("position");
            float explosionRadius = _owner->GetAttributeDataPtr<float>("explosionRadius");
            float explosionForce = _owner->GetAttributeDataPtr<float>("explosionForce");
            if ( explosionForce > 0.0f ) {
                //_world.Explosion(pos, explosionRadius, explosionForce); // TODO: Dispatch event for this
                _manager.destroyEntity(_ownerID);
            } else { // Imploder
				// TODO: All these particles and shit should move to the world
                float implosionDuration = _owner->GetAttributeDataPtr<float>("implosionDuration");
                if ( implosionDuration >= 0.0f ) {
                    ParticleComponent* particleComp = (ParticleComponent*)_manager.getComponent(_ownerID, "Particle");
                    if ( !particleComp ) {
                        particleComp = new ParticleComponent(_ownerID, "BlackHole3D.plist", _manager, _particles);
                        _manager.setComponent(_ownerID, particleComp);
                        printf("Particle sys added to %i\n", _ownerID);
                    }
                    PhysicsComponent* pComp = (PhysicsComponent*)_manager.getComponent(_ownerID, "Physics");
                    if ( pComp ) { _manager.removeComponent(_ownerID, pComp); }

                    if ( implosionDuration > 0.0f ) {
                        implosionDuration -= delta;
                        _owner->GetAttributeDataPtr<float>("implosionDuration") = implosionDuration;
                    }
                    //_locator.Get<Physics>()->Explosion(btVector3(pos.x,pos.y,pos.z), explosionRadius, explosionForce);
                } else if ( implosionDuration < 0.0f ) {
                    _manager.destroyEntity(_ownerID);
                }
            }
        } else if ( std::abs(_timer - 0.2) < 0.05f ) {
            Entity* _owner = _manager.getEntity(_ownerID);
            float explosionForce = _owner->GetAttributeDataPtr<float>("explosionForce");
            if ( explosionForce < 0.0f ) { // Imploder hop up before detonating
                PhysicsComponent* pComp = (PhysicsComponent*)_manager.getComponent(_ownerID, "Physics");
                if ( pComp ) pComp->getRigidBody()->applyCentralImpulse(btVector3(0.0f,0.5f,0.0f));
            }
        }
    }
}

void ExplosiveComponent::activate()
{
    _timer = 2.0f;
}

