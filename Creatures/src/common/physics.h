#ifndef PHYSICS_H
#define PHYSICS_H

namespace Common
{
	//PhysX variables
	extern NxPhysicsSDK*	sdk;
	extern NxScene*			scene;

	//Initialize physics
	void phys_init();
	
};


#endif

