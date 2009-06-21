#ifndef CREATURE_H
#define CREATURE_H

#include "common/sys_includes.h"
#include "project/circuit.h"
#include <vector>

namespace Game
{

using namespace std;

extern void init_creatures();

//A sensor is used to acquire input from the environment for the creature's control network
struct JointSensor : Gate
{
	NxRevoluteJoint* joint;
	
	JointSensor(NxRevoluteJoint* joint_) : joint(joint_) {}
	virtual ~JointSensor() {}
	virtual void update();
};

//An effector is a terminal node in a creature's neural network.  It controls the actuators/motors of the creature's limbs
struct JointEffector : Gate
{
	NxRevoluteJoint* joint;
	float max_force;
	
	JointEffector(NxRevoluteJoint* joint_, float max_f) : joint(joint_), max_force(max_f) {}
	virtual ~JointEffector() {}
	virtual void update();
};

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
	
	//Updates internal state variables
	void update();
	
	//Adds a limb to the object
	void attachPart(BodyPart* part, NxRevoluteJoint* joint, float strength);
	
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
		
	//TODO: Add material properties
	
	//Local control circuit for this creature
	vector<Gate*>				controls;
	vector<Gate*>				sensors;
	vector<Gate*>				effectors;
	vector<Wire*>				wires;
	
private:

	//Initializes the shape given the particular shape descriptor
	void init_shape(NxShapeDesc* shape_desc, const NxMat34& pose);
};

//Creature data
struct Creature
{
	//Constructor/destructor for creature
	Creature();
	~Creature();

	//Draws the critter
	void draw() const;
	
	//Updates the creature
	void update();
	
	//Body information for the creature
	BodyPart*			root;
	vector<BodyPart*>	body;
	
	//Actor group for this creature
	NxActorGroup		group;
};

};

#endif
