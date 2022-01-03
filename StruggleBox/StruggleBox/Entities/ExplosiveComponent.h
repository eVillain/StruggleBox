#ifndef EXPLOSIVE_COMPONENT_H
#define EXPLOSIVE_COMPONENT_H

#include "EntityComponent.h"
#include <memory>

class EntityManager;
class Particles;

class ExplosiveComponent : public EntityComponent
{
public:
    ExplosiveComponent(
		const int ownerID,
		EntityManager& manager,
		Particles& particles);
    ~ExplosiveComponent();
    
    void update(const double delta);

    void activate();
    
private:
	EntityManager& _manager;
	Particles& _particles;
    double _timer;
    double _duration;
};

#endif /* EXPLOSIVE_COMPONENT_H */
