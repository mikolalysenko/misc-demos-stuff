#ifndef CREATURE_H
#define CREATURE_H

#include "common/sys_includes.h"
#include <vector>

namespace Game
{

using namespace std;

extern void init_creatures();

//Body part type
enum BodyPartType
{
	BODY_BOX,
	BODY_SPHERE,
	BODY_CAPSULE,
};

//A creature body part, this is the abstract interface
struct BodyPart
{
	//Box constructor
	BodyPart(
		struct Creature*	owner_,
		const NxVec3&		color_,
		const NxMat34&		pose_,
		const NxVec3&		size_);
	
	//Sphere constructor
	BodyPart(
		struct Creature*	owner_,
		const NxVec3&		color_,
		const NxMat34&		pose_,
		float				radius_);

	//Capsule constructor
	BodyPart(
		struct Creature*	owner_,
		const NxVec3&		color_,
		const NxMat34&		pose_,
		float				radius_,
		float				length_);

	//Virtual interfaces
	~BodyPart();
	
	//Draws the actual body part
	void draw() const;
	
	//Adds a limb to the object
	void attachPart(BodyPart* part, NxRevoluteJoint* joint)
	{
		limbs.push_back(part);
		joints.push_back(joint);
	}
	
	//Attributes common to all body parts
	NxVec3						color;	//Color of the part
	NxActor*					actor;	//Physical actor
	vector<BodyPart*>			limbs;	//Limbs
	vector<NxRevoluteJoint*>	joints;	//Joints
	struct Creature*			owner;	//Creature which owns this part
	
	//Shape parameters
	BodyPartType				shape;
	NxVec3						size;
	float						radius;
	float						length;
		
	//TODO: Add material properties here
	
private:

	//Initializes the shape given the particular shape descriptor
	void init_shape(NxShapeDesc* shape_desc, const NxMat34& pose);
};

//Creature data
struct Creature
{
	//Draws the critter
	void draw() const;
	
	//Body information for the creature
	BodyPart*			root;
	vector<BodyPart*>	body;
};

};

#endif

