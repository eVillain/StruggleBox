#include "CollisionDispatcher.h"
#include "Entity.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/BroadphaseCollision/btDispatcher.h"

CollisionDispatcher::CollisionDispatcher(btCollisionConfiguration* collisionConfiguration) :
btCollisionDispatcher(collisionConfiguration)
{}

bool CollisionDispatcher::needsCollision(
	const btCollisionObject* body0,
	const btCollisionObject* body1)
{
    btAssert(body0);
	btAssert(body1);
        
	if ((!body0->isActive()) && (!body1->isActive()))
		return false;
	else if (!body0->checkCollideWith(body1))
		return false;
	
    Entity* ownerA = (Entity*)body0->getUserPointer();
    Entity* ownerB = (Entity*)body1->getUserPointer();
    if ( ownerA && ownerB ) {
        int IDA = ownerA->GetAttributeDataPtr<int>("ID");
        int ownerIDB = ownerB->GetAttributeDataPtr<int>("ownerID");
        if ( IDA == ownerIDB ) {
            return false;
        }

        int IDB = ownerB->GetAttributeDataPtr<int>("ID");
        int ownerIDA = ownerA->GetAttributeDataPtr<int>("ownerID");
        if ( IDB == ownerIDA ) {
            return false;
        }

    }
    return true;
}

bool CollisionDispatcher::needsResponse(
	const btCollisionObject* body0,
	const btCollisionObject* body1)
{
    //here you can do filtering
	bool hasResponse = (body0->hasContactResponse() && body1->hasContactResponse());
	//no response between two static/kinematic bodies:
	hasResponse = hasResponse && ((!body0->isStaticOrKinematicObject()) ||(!body1->isStaticOrKinematicObject()));
	if ( hasResponse == false ) return false;
    
    Entity* ownerA = (Entity*)body0->getUserPointer();
    Entity* ownerB = (Entity*)body1->getUserPointer();
    if ( ownerA && ownerB ) {
        int IDA = ownerA->GetAttributeDataPtr<int>("ID");
        int ownerIDB = ownerB->GetAttributeDataPtr<int>("ownerID");
        if ( IDA == ownerIDB ) {
            return false;
        }
        
        int IDB = ownerB->GetAttributeDataPtr<int>("ID");
        int ownerIDA = ownerA->GetAttributeDataPtr<int>("ownerID");
        if ( IDB == ownerIDA ) {
            return false;
        }
        
    }
    return hasResponse;
}
