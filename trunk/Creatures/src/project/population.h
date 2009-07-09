#ifndef POPULATION_H
#define POPULATION_H

#include <iostream>
#include <string>

#include "project/creature.h"
#include "project/genotype.h"

namespace Game
{

//Interface for a fitness test
struct FitnessTest
{
	Creature* creature;
	NxVec3 base_position;
	
	double current_time, round_time, rest_time, fitness;
	
	double max_height;
	
	
	FitnessTest() {}
	FitnessTest(
		float round_time_,
		float rest_time_) : 
			creature(NULL),
			round_time(round_time_),
			rest_time(rest_time_) {}

	virtual void start_test(Genotype& genes);
	virtual bool update();
	virtual void draw();
};

//A population of creatures
struct Population
{
	vector< pair<float,Genotype> >	species;
	vector< pair<float,Genotype> >	best_species;
	
	FitnessTest*	tester;
	
	//Constructors
	Population() {}
	Population(
		int num_creatures,
		int num_high_scores,
		FitnessTest* test);
	
	//Updates the population
	void update();
	void draw();

	void save(ostream& os);
	static void load(istream& is);
	
private:
	int 	current_test;

	///Generates a new creature
	void next_round();
};

};

#endif

