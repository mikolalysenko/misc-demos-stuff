#include "common/sys_includes.h"
#include "common/physics.h"

#include "project/creature.h"

#include <iostream>
#include <vector>

using namespace std;
using namespace Common;

namespace Game
{

GLint	shape_lists;

void init_creatures()
{
	shape_lists = glGenLists(3);
	
	//Generate box list
	glNewList(shape_lists, GL_COMPILE);
	glBegin(GL_QUADS);
		glVertex3f(1,1,1);
		glVertex3f(-1,1,1);
		glVertex3f(-1,-1,1);
		glVertex3f(1,-1,1);

		glVertex3f(1,1,1);
		glVertex3f(-1,1,1);
		glVertex3f(-1,1,-1);
		glVertex3f(1,1,-1);

		glVertex3f(1,1,1);
		glVertex3f(1,-1,1);
		glVertex3f(1,-1,-1);
		glVertex3f(1,1,-1);

		glVertex3f(-1,1,1);
		glVertex3f(-1,-1,1);
		glVertex3f(-1,-1,-1);
		glVertex3f(-1,1,-1);

		glVertex3f(-1,-1,1);
		glVertex3f(1,-1,1);
		glVertex3f(1,-1,-1);
		glVertex3f(-1,-1,-1);

		glVertex3f(1,1,-1);
		glVertex3f(1,-1,-1);
		glVertex3f(-1,-1,-1);
		glVertex3f(-1,1,-1);
	glEnd();
	glEndList();
	
	
	//Generate sphere list
	glNewList(shape_lists+1, GL_COMPILE);
	glBegin(GL_TRIANGLES);
		
		float d_theta = M_PI/32.f,
				d_phi = M_PI/16.f;
		
		for(float theta=-M_PI; theta<=M_PI; theta+=d_theta)
		for(float phi=-M_PI/2.; phi<=M_PI/2.; phi+=d_phi)
		{
			
			glVertex3f(cos(theta) * cos(phi), sin(theta) * cos(phi), sin(phi));
			glVertex3f(cos(theta+d_theta) * cos(phi), sin(theta+d_theta) * cos(phi), sin(phi));
			glVertex3f(cos(theta) * cos(phi+d_phi), sin(theta) * cos(phi+d_phi), sin(phi+d_phi));
			
			glVertex3f(cos(theta+d_theta) * cos(phi), sin(theta+d_theta) * cos(phi), sin(phi));
			glVertex3f(cos(theta) * cos(phi+d_phi), sin(theta) * cos(phi+d_phi), sin(phi+d_phi));
			glVertex3f(cos(theta+d_theta) * cos(phi+d_phi), sin(theta+d_theta) * cos(phi+d_phi), sin(phi+d_phi));
		}
		
	glEnd();
	glEndList();
	
	
	//Generate Capsule list
	glNewList(shape_lists+2, GL_COMPILE);
	glBegin(GL_TRIANGLES);
		//TODO: Capsule drawing
	glEnd();
	glEndList();
}

//Retrieves information about relative joint information
void JointSensor::update()
{
	NxActor* a;
	NxActor* b;
	joint->getActors(&a, &b);
	
	NxQuat qa = a->getGlobalOrientationQuat(),
			qb = b->getGlobalOrientationQuat();
			
	qa.invert();
	qb *= qa;
	
	write(0, qb.x);
	write(1, qb.y);
	write(2, qb.z);
	write(3, qb.w);
}

//A joint effector drives the creature's motion
void JointEffector::update()
{
	NxVec3 axis = NxVec3(read(3), read(2), read(1));
	if(axis.magnitude() <= 1e-4)
		axis = NxVec3(0, 0, 1);
	axis.normalize();
	
	NxQuat q;
	q.fromAngleAxis(read(0), axis);

	static_cast<NxD6Joint*>(joint)->setDriveOrientation(q);
}

//Copied from SDK
static NxCCDSkeleton* CreateCCDSkeleton (NxVec3 size)
{
	NxU32 triangles[3 * 12] = { 
		0,1,3,
		0,3,2,
		3,7,6,
		3,6,2,
		1,5,7,
		1,7,3,
		4,6,7,
		4,7,5,
		1,0,4,
		5,1,4,
		4,0,2,
		4,2,6
	};

	NxVec3 points[8];

	//static mesh
	points[0].set( size.x, -size.y, -size.z);
	points[1].set( size.x, -size.y,  size.z);
	points[2].set( size.x,  size.y, -size.z);
	points[3].set( size.x,  size.y,  size.z);

	points[4].set(-size.x, -size.y, -size.z);
	points[5].set(-size.x, -size.y,  size.z);
	points[6].set(-size.x,  size.y, -size.z);
	points[7].set(-size.x,  size.y,  size.z);

	NxSimpleTriangleMesh stm;
	stm.numVertices = 8;
	stm.numTriangles = 6*2;
	stm.pointStrideBytes = sizeof(NxVec3);
	stm.triangleStrideBytes = sizeof(NxU32)*3;

	stm.points = points;
	stm.triangles = triangles;
	stm.flags |= NX_MF_FLIPNORMALS;
	return sdk->createCCDSkeleton(stm);
}


//Initialize the body part
void BodyPart::init_shape(NxShapeDesc* foo, const NxMat34& tpose)
{
	actor = NULL;
	skeleton = NULL;

	//Check for collision
	
	NxMat34 pose = tpose;

/*
	NxBox box(pose.t, size*.6, pose.M);
	if(scene->checkOverlapOBB(box))
	{
		return;
	}
*/
	
	NxBoxShapeDesc shape_desc;
	shape_desc.dimensions = size;
	shape_desc.ccdSkeleton = CreateCCDSkeleton(size*0.6f);
	shape_desc.shapeFlags |= NX_SF_DYNAMIC_DYNAMIC_CCD; //Activate dynamic-dynamic CCD for 

	// Create body
	NxBodyDesc bodyDesc;
	bodyDesc.angularDamping	= 0.5f;

	//Set up parameters
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&shape_desc);
	actorDesc.body			= &bodyDesc;
	actorDesc.density		= 10.0f;
	actorDesc.globalPose	= pose;
	actorDesc.group			= owner->group;
	
	//Create the actor
	actor = scene->createActor(actorDesc);
	if(actor == NULL)
		return;
		
	actor->userData = (void*)this;
	
	//TODO: Attach touch sensor
}


//Box constructor
BodyPart::BodyPart(
	Creature*			owner_,
	const NxVec3&		color_,
	const NxMat34&		pose_,
	const NxVec3&		size_) 
		: color(color_), owner(owner_), shape(BODY_BOX), size(size_)
{
	//Create box shape descriptor
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = NxVec3(size.x, size.y, size.z);

	//Initialize shape
	init_shape(&boxDesc, pose_);
}

//Sphere contructor
BodyPart::BodyPart(
	Creature*			owner_,
	const NxVec3&		color_,
	const NxMat34&		pose_,
	float				radius_)
			: color(color_), owner(owner_), shape(BODY_SPHERE), radius(radius_)
{
	//Create box shape descriptor
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;

	//Initialize shape
	init_shape(&sphereDesc, pose_);
}


//Body part destructor
BodyPart::~BodyPart()
{
	//Need to clean up joints first
	for(int i=0; i<(int)joints.size(); i++)
		scene->releaseJoint(*joints[i]);
		
	for(int i=0; i<(int)controls.size(); i++)
		delete controls[i];
	for(int i=0; i<(int)effectors.size(); i++)
		delete effectors[i];
	for(int i=0; i<(int)sensors.size(); i++)
		delete sensors[i];
	for(int i=0; i<(int)wires.size(); i++)
		delete wires[i];

	//Then release actor
	if(actor != NULL)
		scene->releaseActor(*actor);

	if(skeleton != NULL)
		sdk->releaseCCDSkeleton(*skeleton);
}

//Draws a body part
void BodyPart::draw() const
{
	//Set up matrix
	glPushMatrix();
	float glMat[16];
	actor->getGlobalPose().getColumnMajor44(glMat);	
	glMultMatrixf(glMat);
	
	//Set appropriate scale
	switch(shape)
	{
		case BODY_BOX:
			glScalef(size.x, size.y, size.z);
		break;
		
		case BODY_SPHERE:
			glScalef(radius, radius, radius);
		break;
		
		case BODY_CAPSULE:
			glScalef(radius, length, radius);
		break;
	}		
	
	//Set color
	glColor3f(color.x, color.y, color.z);
	
	//Draw shape
	glCallList(shape_lists + (int)shape);
	
	glPopMatrix();
}

//Update each control
void BodyPart::update()
{
	for(int i=0; i<(int)sensors.size(); i++)
		sensors[i]->update();
	for(int i=0; i<(int)controls.size(); i++)
		controls[i]->update();
	for(int i=0; i<(int)effectors.size(); i++)
		effectors[i]->update();
}

void BodyPart::attachPart(BodyPart* part, NxJoint* joint, float strength)
{
	limbs.push_back(part);
	joints.push_back(joint);
	effectors.push_back(new JointEffector(joint, strength));
	sensors.push_back(new JointSensor(joint));
}


//Acquire group
Creature::Creature()
{
	group = get_group();
}

//Release stuff
Creature::~Creature()
{
	for(int i=0; i<(int)body.size(); i++)
		delete body[i];
	release_group(group);
}


//Draw a creature
void Creature::draw() const
{
	for(int i=0; i<(int)body.size(); i++)
		body[i]->draw();
}

//Updates creature (incl. logic, contact sensors, etc.)
void Creature::update()
{
	for(int i=0; i<(int)body.size(); i++)
		body[i]->update();
}

};

