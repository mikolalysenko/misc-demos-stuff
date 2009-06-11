#ifndef CREATURE_H
#define CREATURE_H

#include "common/sys_includes.h"
#include <vector>

namespace Game
{

using namespace std;

//A creature body part, this is the abstract interface
struct BodyPart
{
	BodyPart();
	BodyPart(NxActor* actor_) : actor(actor_) {}

	//Virtual interfaces
	virtual ~BodyPart();
	virtual void draw() const;
	
	//Adds a limb to the object
	void attachPart(BodyPart* part)  { limbs.push_back(part); }
	
	//Attributes common to all body parts
	NxVec3				color;	//Color of the part
	NxActor*			actor;	//Physical actor
	vector<BodyPart*>	limbs;	//Limbs
	struct Creature*	owner;	//Creature which owns this part
};

//A box body part
struct BoxPart : public BodyPart
{
	virtual ~BoxPart();
	virtual void draw() const;

	//The size of the box
	NxVec3	size;
};

//A capsule body part
struct CapsulePart : public BodyPart
{
	virtual ~CapsulePart();
	virtual void draw() const;
	
	float length, radius;
};

//A spherical body part
struct SpherePart : public BodyPart
{
	virtual ~SpherePart();
	virtual void draw() const;
	
	float radius;
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

