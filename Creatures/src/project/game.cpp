//Engine stuff
#include "common/sys_includes.h"
#include "common/input.h"
#include "common/physics.h"

//Project stuff
#include "project/game.h"
#include "project/creature.h"
#include "project/genotype.h"

//STL stuff
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace Common;

namespace Game
{

//Config variables
int YRes			= 480;
int XRes			= 640;
bool fullscreen		= false;
float fov			= 45.0f;
float z_near		= 0.5f;
float z_far			= 1200.0f;
float delta_t		= 1. / 60.;



//Scenario stuff
int floor_height = -10; 


Creature* critter;

//The scenario
void init_scenario()
{
	// Set default material (values taken from boxes demo, need to mess with this)
	NxMaterial* defaultMaterial = scene->getMaterialFromIndex(0);
	defaultMaterial->setRestitution(0.f);
	defaultMaterial->setStaticFriction(.5f);
	defaultMaterial->setDynamicFriction(.5f);

	// Create ground plane
	NxPlaneShapeDesc planeDesc;
	planeDesc.normal = NxVec3(0, 1, 0);
	planeDesc.d = floor_height;
	
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&planeDesc);
	scene->createActor(actorDesc);
}


//Initialization
void init()
{
	init_creatures();

	//Set up the environment
	init_scenario();

/*
	//Create a test creature
	critter = new Creature();
	NxMat34 pose;
	pose.t = NxVec3(0, 20, -50);
	critter->root = new BodyPart(
		critter,
		NxVec3(1,.5,1),
		pose,
		NxVec3(.1, 5, .2));
	critter->body.push_back(critter->root);
*/
	
	try
	{
		ifstream fin("data/test.dna");
		Genotype test = Genotype::load(fin);
		test.save(cout);
		
		NxMat34 frame;
		frame.id();
		frame.t = NxVec3(0, 0, -50);
		
		critter = test.createCreature(frame);
	}
	catch(const char *err)
	{
		cerr << "Error loading creature: " << err << endl;	
	}
	catch(const string err)
	{
		cerr << "Error loading creature: " << err << endl;	
	}
}



//Update the game state
void update()
{
}


//Draw stuff
void draw()
{

	//Draw floor
	glBegin(GL_LINES);
	glColor3f(1,1,1);
	for(int i=-50; i<=50; i++)
	{
		glVertex3f(i, floor_height, -50);
		glVertex3f(i, floor_height, 50);

		glVertex3f(-50, floor_height, i);
		glVertex3f(50, floor_height, i);
		
	}
	glEnd();
	
	//Draw critter
	critter->draw();
}


//Handle overlays
void overlays()
{
}

};

