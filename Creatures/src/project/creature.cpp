#include "common/sys_includes.h"
#include "common/physics.h"

#include "project/creature.h"

#include <iostream>
#include <vector>

using namespace std;
using namespace Common;

namespace Game
{

//Draws a body part
void BodyPart::draw() const
{
	glPushMatrix();
	float glMat[16];
	actor->getGlobalPose().getColumnMajor44(glMat);
	
	glMultMatrixf(glMat);
	glColor3f(color.x, color.y, color.z);
	
	/*
	glBegin(GL_QUADS);
		glVertex3f(size.x,size.y,size.z);
		glVertex3f(-size.x,size.y,size.z);
		glVertex3f(-size.x,-size.y,size.z);
		glVertex3f(size.x,-size.y,size.z);

		glVertex3f(size.x,size.y,size.z);
		glVertex3f(-size.x,size.y,size.z);
		glVertex3f(-size.x,size.y,-size.z);
		glVertex3f(size.x,size.y,-size.z);

		glVertex3f(size.x,size.y,size.z);
		glVertex3f(size.x,-size.y,size.z);
		glVertex3f(size.x,-size.y,-size.z);
		glVertex3f(size.x,size.y,-size.z);

		glVertex3f(-size.x,size.y,size.z);
		glVertex3f(-size.x,-size.y,size.z);
		glVertex3f(-size.x,-size.y,-size.z);
		glVertex3f(-size.x,size.y,-size.z);

		glVertex3f(-size.x,-size.y,size.z);
		glVertex3f(size.x,-size.y,size.z);
		glVertex3f(size.x,-size.y,-size.z);
		glVertex3f(-size.x,-size.y,-size.z);

		glVertex3f(size.x,size.y,-size.z);
		glVertex3f(size.x,-size.y,-size.z);
		glVertex3f(-size.x,-size.y,-size.z);
		glVertex3f(-size.x,size.y,-size.z);
	glEnd();
	*/
	
	glPopMatrix();
}


void Creature::draw() const
{
	for(int i=0; i<body.size(); i++)
		body[i]->draw();
}

};

