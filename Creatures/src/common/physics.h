#ifndef PHYSICS_H
#define PHYSICS_H

namespace Common
{
	//PhysX variables
	extern NxPhysicsSDK*	sdk;
	extern NxScene*			scene;

	//Initialize physics
	void phys_init();
	
	//Actor group stuff
	NxActorGroup get_group();
	void release_group(NxActorGroup);
};


#endif

