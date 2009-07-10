#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <utility>

#include "common/sys_includes.h"
#include "common/physics.h"

#include "project/population.h"
#include "project/creature.h"
#include "project/genotype.h"
#include "project/mutation.h"


using namespace std;
using namespace Common;

namespace Game
{

//Create a creature in the middle of space
void FitnessTest::start_test(Genotype& genes)
{
	//genes.save(cout);
	
	cout << "Generating creature..." << endl;
	NxMat34 start_pos;
	start_pos.id();
	creature = genes.createCreature(start_pos);
	base_position = NxVec3(0.,0.,0.);
	current_time = 0.;
	max_height = -1e10;
}

//Update the creature's position
bool FitnessTest::update()
{
	if(creature == NULL)
		return false;

	creature->update();
	NxMat34 pose = creature->get_pose();
	NxVec3 pos = pose.t;
	
	if(current_time <= rest_time)
	{
		base_position = pos;
	}
	
	max_height = max(max_height, (double)pos.y);
	
	//Update fitness function
	fitness = (pos - base_position) .magnitude() + 1e-3;
	
	for(int i=0; i<(int)creature->body.size(); i++)
	{
		BodyPart *p = creature->body[i];
		for(int j=0; j<(int)p->joints.size(); j++)
		{
			if(p->joints[j]->getState() == NX_JS_BROKEN)
			{
				fitness = 0.0001;
				delete creature;
				creature = NULL;
				return false;
			}
		}
	}
	
	
	current_time += 1.;

	if(current_time > round_time || creature->body.size() <= 1)
	{
		delete creature;
		creature = NULL;
		return false;
	}
	
	return true;
	
}

void FitnessTest::draw()
{
	if(creature != NULL)
		creature->draw();
}


//Population constructor
Population::Population(
		int num_creatures_,
		int num_high_scores_,
		FitnessTest* tester_)
	: species(num_creatures_),
	  best_species(num_high_scores_),
	  tester(tester_),
	  current_test(0)
{
	
	//Generate a random intial population
	for(int i=0; i<(int)species.size(); i++)
	{
		species[i] = make_pair(0., randomCreature());
	}

/*
	//Recover best creature
	ifstream best("data/best.dna");
	species[0] = make_pair(0., Genotype::load(best));
	best.close();
*/

	for(int i=0; i<(int)best_species.size(); i++)
	{
		best_species[i] = species[0];
	}
}
	
//Updates the population
void Population::update()
{
	if(!tester->update())
	{
		if(current_test > 0)
		{
			species[current_test-1].first = tester->fitness;
			cout << "Fitness = " << tester->fitness << endl;
			
			//Print stats
			NxSceneStats stats;
			scene->getStats(stats);
			cout << "Num actors = " << stats.numActors << endl
				 << "Num joints = " << stats.numJoints << endl
				 << "Num contacts = " << stats.numContacts << endl;
		}
	
		if(current_test >= (int)species.size())
		{
			next_round();
			current_test = 0;
		}
		
		//Start new test
		tester->start_test(species[current_test].second);
		current_test++;
	}
}

void Population::next_round()
{
	//Normalize scores, store high scoring creature
	cout << "Total scores:" << endl;
	float s = 0.;
	for(int i=0; i<(int)species.size(); i++)
	{
		cout << species[i].first << endl;
	
		s += species[i].first;
		if(species[i].first > best_species[0].first)
		{
			best_species[0] = species[i];
			sort(best_species.begin(), best_species.end());
		}
	}
	
	cout << "Top score:" << endl;
	cout << best_species[best_species.size()-1].first << endl;
	
	ofstream best("data/best.dna");
	best_species[best_species.size()-1].second.save(best);
	
	//Generate new population using stupid rule
	vector< pair<float,Genotype> >  next(species.size());
	
	for(int i=0; i<(int)species.size(); i++)
	{
		float r = drand48() * s;
		for(int j=0; j<(int)species.size(); j++)
		{
			r -= species[j].first;
			if(r <= 0.)
			{
				cout << "Fitness: " << species[j].first << endl;
			
				next[i] = make_pair(0., species[j].second);
				mutate(next[i].second);
				break;
			}
		}
	}
	
	//Set new species
	species = next;
}

void Population::draw()
{
	tester->draw();
}

void Population::save(ostream& os)
{
}

void Population::load(istream& is)
{
}



};
