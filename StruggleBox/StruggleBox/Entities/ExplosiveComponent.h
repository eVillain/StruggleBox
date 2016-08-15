#ifndef EXPLOSIVE_COMPONENT_H
#define EXPLOSIVE_COMPONENT_H

#include "EntityComponent.h"
#include <memory>

class EntityManager;
class World3D;
class Particles;

class ExplosiveComponent : public EntityComponent
{
public:
    ExplosiveComponent(
		const int ownerID,
		std::shared_ptr<EntityManager> manager,
		std::shared_ptr<World3D> world,
		std::shared_ptr<Particles> particles);
    ~ExplosiveComponent();
    
    void update(const double delta);

    void activate();
    
private:
	std::shared_ptr<EntityManager> _manager;
	std::shared_ptr<World3D> _world;
	std::shared_ptr<Particles> _particles;
    double _timer;
    double _duration;
};

#endif /* EXPLOSIVE_COMPONENT_H */
