#include "common/sys_includes.h"
#include "physics.h"

using namespace std;


namespace Common
{
	//PhysX variables
	NxPhysicsSDK*	sdk;
	NxScene*		scene;
	
	void phys_init()
	{
		//Create the SDK
		phys_sdk = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
		if(phys_sdk == NULL)
		{
			printf("Failed to initialize PhysX\n");
			exit(1);
		}
		
		//Create the scene
		NxSceneDesc scene_desc;
		scene_desc.gravity = NxVec3(0.f, -9.81f, 0.f);
		scene = sdk->createScene(scene_desc);
	}
};
