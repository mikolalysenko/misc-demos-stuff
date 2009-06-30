//Engine stuff
#include "common/sys_includes.h"
#include "common/input.h"
#include "common/physics.h"

//Project stuff
#include "project/game.h"
#include "project/creature.h"
#include "project/genotype.h"
#include "project/mutation.h"
#include "project/population.h"

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


float lifetime		= 200.;


NxMat34 camera;



//Scenario stuff
int floor_height = -10; 


//Creature* critter;
//Genotype test;

Population*	population;


//The scenario
void init_scenario()
{
	camera.id();

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
	
	/*
	try
	{
		ifstream fin("data/test2.dna");
		test = Genotype::load(fin);
		test.save(cout);
		
		NxMat34 frame;
		frame.id();
		frame.t = NxVec3(0, 0, 0);
		critter = test.createCreature(frame);
		
		assert(critter != NULL);
	}
	catch(const char *err)
	{
		cerr << "Error loading creature: " << err << endl;	
	}
	catch(const string err)
	{
		cerr << "Error loading creature: " << err << endl;	
	}
	*/
	
	FitnessTest* tester = new FitnessTest();
	
	population = new Population(
		20,
		10,
		1000.,
		tester);
}



//Update the game state
void update()
{
/*
	critter->update();
	
	lifetime -= 1.;
	
	if(lifetime <= 0.)
	{
		cout << "Reset!" << endl;
		lifetime = 300.;
	
		//Reset creature
		delete critter;
		
		cout << "Creature deleted." << endl;

		mutate(test);

		cout << "Mutation complete; new genotype: " << endl;
		
		test.save(cout);


		cout << "Building new creature" << endl;

		NxMat34 frame;
		frame.id();
		frame.t = NxVec3(0, 0, 0);
		critter = test.createCreature(frame);
	}
*/

	population->update();


	NxMat34 trans;
	trans.id();

	if(key_down(SDLK_UP))
	{
		trans.t += NxVec3(0,0,0.25);
	}
	if(key_down(SDLK_DOWN))
	{
		trans.t += NxVec3(0,0,-0.25);
	}
	if(key_down(SDLK_RIGHT))
	{
		trans.t += NxVec3(-0.25,0.,0.);
	}
	if(key_down(SDLK_LEFT))
	{
		trans.t += NxVec3(0.25,0.,0.);
	}
	if(key_down('a'))
	{
		trans.t += NxVec3(0.,-.25,0.);
	}
	if(key_down('z'))
	{
		trans.t += NxVec3(0.,.25,0.);
	}

	
	camera = camera * trans;
}


//Draw stuff
void draw()
{
	float mat[16];
	camera.getColumnMajor44(mat);
	glMultMatrixf(mat);

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
	//critter->draw();
	population->draw();
}


//Handle overlays
void overlays()
{
}

};

