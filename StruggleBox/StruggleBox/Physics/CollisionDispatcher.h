//
//  CollisionDispatcher.h
//  Ingenium
//
//  Created by The Drudgerist on 05/01/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#ifndef NGN_COLLISION_DISPATCHER_H
#define NGN_COLLISION_DISPATCHER_H

#include "btBulletDynamicsCommon.h"

class CollisionDispatcher : public btCollisionDispatcher {
    
public:
    CollisionDispatcher (btCollisionConfiguration* collisionConfiguration);

    bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1);
    bool needsResponse(const btCollisionObject* body0, const btCollisionObject* body1);

};
#endif /* defined(NGN_COLLISION_DISPATCHER_H) */
