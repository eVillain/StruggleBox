#ifndef COLLISION_DISPATCHER_H
#define COLLISION_DISPATCHER_H

#include "btBulletDynamicsCommon.h"

class CollisionDispatcher : public btCollisionDispatcher
{    
public:
    CollisionDispatcher(btCollisionConfiguration* collisionConfiguration);

    bool needsCollision(
		const btCollisionObject* body0,
		const btCollisionObject* body1);

    bool needsResponse(
		const btCollisionObject* body0,
		const btCollisionObject* body1);

};
#endif /* COLLISION_DISPATCHER_H */
