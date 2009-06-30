#include <algorithm>
#include <vector>
#include <string>
#include <iostream>

#include "project/population.h"
#include "project/creature.h"
#include "project/genotype.h"
#include "project/mutation.h"


using namespace std;

namespace Game
{

//Create a creature in the middle of space
void FitnessTest::start_test(Genotype* genes)
{
	cout << "Starting test" << endl;
	
	genes->save(cout);
	
	NxMat34 start_pos;
	start_pos.id();
	creature = genes->createCreature(start_pos);
}

void FitnessTest::stop_test()
{
	cout << "Stopping test" << endl;
	delete creature;
}

//Update the creature's position
void FitnessTest::update()
{
	cout << "Updating" << endl;
	creature->update();
}

void FitnessTest::draw()
{
	creature->draw();
}

float FitnessTest::get_fitness()
{
	NxMat34 pose = creature->get_pose();
	return pose.t.magnitude();
}


//Population constructor
Population::Population(
		int num_creatures_,
		int num_high_scores_,
		float round_time_,
		FitnessTest* tester_)
	: species(num_creatures_),
	  best_species(num_high_scores_),
	  tester(tester_),
	  max_round_time(round_time_),
	  round_time(0.),
	  current_test(-1)
{
	//Generate a random intial population
	for(int i=0; i<(int)species.size(); i++)
	{
		species[i] = make_pair(0., new Genotype(randomCreature()));
	}
	
	for(int i=0; i<(int)best_species.size(); i++)
	{
		best_species[i] = species[0];
	}
}
	
//Updates the population
void Population::update()
{
	if(round_time <= 0.)
	{
		if(current_test > 0)
		{
			species[current_test-1].first = tester->get_fitness();
			tester->stop_test();
		}
	
		if(current_test >= (int)species.size())
		{
			next_round();
			current_test = 0;
		}
		
		//Start new test
		tester->start_test(species[current_test].second);
		current_test++;
		round_time = max_round_time;
	}

	tester->update();
}

void Population::next_round()
{
	//Normalize scores, store high scoring creature
	float s = 0.;
	for(int i=0; i<(int)species.size(); i++)
	{
		s += species[i].first;
		if(species[i].first > best_species[0].first)
		{
			best_species[0] = species[i];
			sort(best_species.begin(), best_species.end());
		}
	}
	
	//Generate new population using stupid rule
	vector< pair<float,Genotype*> >  next(species.size());
	
	for(int i=0; i<(int)species.size(); i++)
	{
		float r = drand48() * s;
		for(int j=0; j<(int)species.size(); j++)
		{
			r -= species[j].first;
			if(r <= 0.)
			{
				next[i] = make_pair(0., species[j].second);
				mutate(*next[i].second);
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
