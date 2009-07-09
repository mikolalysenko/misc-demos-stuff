#include "common/sys_includes.h"
#include "physics.h"
#include <vector>

using namespace std;


namespace Common
{
	//PhysX variables
	NxPhysicsSDK*	sdk;
	NxScene*		scene;
	
	//Actor group stuff
	vector<NxActorGroup> actor_groups;
	
	void phys_init()
	{
		//Create the SDK
		sdk = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
		if(sdk == NULL)
		{
			printf("Failed to initialize PhysX\n");
			exit(1);
		}
		
		//Create the scene
		NxSceneDesc scene_desc;
		//scene_desc.setToDefault();
		scene_desc.gravity = NxVec3(0.f, -9.81f, 0.f);
		scene = sdk->createScene(scene_desc);
		
		
		//Just fill with set of numbers
		actor_groups.resize(65535);
		for(int i=0; i<actor_groups.size(); i++)
			actor_groups[i] = i+1;
	}
	
	NxActorGroup get_group()
	{
		NxActorGroup res = actor_groups[actor_groups.size()-1];
		actor_groups.pop_back();
		return res;
	}
	
	void release_group(NxActorGroup group)
	{
		actor_groups.push_back(group);
	}

	
};
